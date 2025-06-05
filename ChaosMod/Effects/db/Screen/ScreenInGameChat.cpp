#include <stdafx.h>

#include <queue>

#include "Memory/Hooks/GetLabelTextHook.h"

#include "Components/Voting.h"
#include "Util/Chat/ChatMessage.h"
#include "Util/Events.h"
#include "Util/Random.h"

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR CHAOS_EVENT_LISTENER(Voting::OnNewMessage) m_OnNewMessageListener;
CHAOS_VAR CHAOS_EVENT_LISTENER(Voting::OnMessageDelete) m_OnMessageDeleteListener;
CHAOS_VAR CHAOS_EVENT_LISTENER(Voting::OnUserBan) m_OnUserBanListener;

struct Message
{
	ChatMessage m_Message;
	int m_ReceivedTimestamp;
	int m_DispatchedTimestamp = -1;
	int m_ThefeedId           = -1;
	bool m_Banned             = false;

	Message(ChatMessage message, int timestamp) : m_Message(message), m_ReceivedTimestamp(timestamp)
	{
	}
};

CHAOS_VAR std::list<Message> messageQueue;
CHAOS_VAR std::list<Message> dispatchedQueue;

CHAOS_VAR std::map<std::string, std::string> usercolors;

void encode(std::string &data)
{
	std::string buffer;
	buffer.reserve(data.size());
	for (size_t pos = 0; pos != data.size(); ++pos)
	{
		switch (data[pos])
		{
		case '&':
			buffer.append("&amp;");
			break;
		case '\"':
			buffer.append("&quot;");
			break;
		case '\'':
			buffer.append("&apos;");
			break;
		case '<':
			buffer.append("&lt;");
			break;
		case '>':
			buffer.append("&gt;");
			break;
		default:
			buffer.append(&data[pos], 1);
			break;
		}
	}
	data.swap(buffer);
}

static void RemoveMessageFromDisplay(Message message)
{
	THEFEED_REMOVE_ITEM(message.m_ThefeedId);
}

static void AddMessageToQueue(ChatMessage message)
{
	auto m = message.m_Message;
	if (m.size() == 1 && m[0] >= '1' && m[0] <= '9')
		return;

	messageQueue.push_back(Message(message, GetTickCount64()));
}

static void RemoveMessageById(std::string id)
{
	for (auto &&m : messageQueue)
		if (m.m_Message.m_Id == id)
			m.m_Banned = true;

	for (auto &&m : dispatchedQueue)
		if (m.m_Message.m_Id == id)
		{
			m.m_Banned = true;
			RemoveMessageFromDisplay(m);
		}
}

static void RemoveMessagesByUserId(std::string userid)
{
	for (auto &&m : messageQueue)
		if (m.m_Message.m_Userstate.m_Userid == userid)
			m.m_Banned = true;

	for (auto &&m : dispatchedQueue)
		if (m.m_Message.m_Userstate.m_Userid == userid)
		{
			m.m_Banned = true;
			RemoveMessageFromDisplay(m);
		}
}

static int DisplayMessage(ChatMessage message)
{
	auto m = message.m_Message;
	encode(m);

	auto userid = message.m_Userstate.m_Userid;
	auto color  = message.m_Userstate.m_ColorHex;
	if (color.empty())
	{
		if (usercolors.contains(userid))
		{
			color = usercolors[userid];
		}
		else
		{
			std::vector<std::string> colors = { "#FF0000", "#0000FF", "#00FF00", "#B22222", "#FF7F50",
				                                "#9ACD32", "#FF4500", "#2E8B57", "#DAA520", "#D2691E",
				                                "#5F9EA0", "#1E90FF", "#FF69B4", "#8A2BE2", "#00FF7F" };

			color                           = colors[g_Random.GetRandomInt(0, 14)];
		}
	}

	usercolors[userid] = color;

	auto username = message.m_Userstate.m_Username;
	auto displayName = message.m_Userstate.m_DisplayName;

	// hack - should've checked for displayName.toLower() != username, but I'm too lazy
	if (displayName.size() != username.size())
		displayName = username;

	std::ostringstream oss;
	oss << "<font color='" << color << "'>" << displayName << ": " << "</font>" << m;

	Hooks::AddCustomLabel(message.m_Id, oss.str());

	PLAY_SOUND_FRONTEND(-1, "Notification", "Phone_SoundSet_Trevor", true);

	BEGIN_TEXT_COMMAND_THEFEED_POST(message.m_Id.c_str());
	return END_TEXT_COMMAND_THEFEED_POST_TICKER(false, false);
}

static void OnStart()
{
	if (ComponentExists<Voting>())
	{
		m_OnNewMessageListener.Register(GetComponent<Voting>()->OnNewMessage,
		                                [&](const ChatMessage &message) { AddMessageToQueue(message); });
		m_OnMessageDeleteListener.Register(GetComponent<Voting>()->OnMessageDelete,
		                                   [&](const ChatMessageDeletion &deletion)
		                                   { RemoveMessageById(deletion.m_MessageId); });
		m_OnUserBanListener.Register(GetComponent<Voting>()->OnUserBan,
		                             [&](const ChatUserBan &userBan) { RemoveMessagesByUserId(userBan.m_Userid); });
	}
}

static void OnTick()
{
	int curTick          = GetTickCount64();

	int lastDispatchTick = !dispatchedQueue.empty() ? dispatchedQueue.back().m_DispatchedTimestamp : 0;

	// 300 ms cooldown between notifications, otherwise thefeed breaks entirely
	if (curTick - lastDispatchTick < 300)
		return;

	// discard old messages if they clog up the queue
	while ((messageQueue.size() > 3 && curTick - messageQueue.front().m_ReceivedTimestamp > 2000)
	       || (!messageQueue.empty() && messageQueue.front().m_Banned))
		messageQueue.pop_front();

	if (!messageQueue.empty())
	{
		Message message = messageQueue.front();
		messageQueue.pop_front();

		message.m_ThefeedId           = DisplayMessage(message.m_Message);

		message.m_DispatchedTimestamp = curTick;

		dispatchedQueue.push_back(message);
	}

	while (dispatchedQueue.size() > 10)
	{
		RemoveMessageFromDisplay(dispatchedQueue.front());
		dispatchedQueue.pop_front();
	}
}

static void OnStop()
{
	if (ComponentExists<Voting>())
	{
		m_OnNewMessageListener.Unregister(GetComponent<Voting>()->OnNewMessage);
		m_OnMessageDeleteListener.Unregister(GetComponent<Voting>()->OnMessageDelete);
		m_OnUserBanListener.Unregister(GetComponent<Voting>()->OnUserBan);
	}

	messageQueue.clear();
	dispatchedQueue.clear();
	usercolors.clear();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick,
	{
		.Name = "In-Game Chat",
		.Id = "screen_ingame_chat",
		.IsTimed = true,
		.ConditionType = EffectConditionType::SilentVotingEnabled
	}
);