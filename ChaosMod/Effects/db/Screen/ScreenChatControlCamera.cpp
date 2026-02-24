#include <stdafx.h>

#include <queue>

#include "Memory/Hooks/GetLabelTextHook.h"

#include "Components/SplashTexts.h"
#include "Components/Voting.h"
#include "Util/Chat/ChatMessage.h"
#include "Util/Events.h"
#include "Util/ScriptText.h"
#include "Util/Text.h"

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR CHAOS_EVENT_LISTENER(Voting::OnNewMessage) m_OnNewMessageListener;

CHAOS_VAR const float ANGLE_INCREMENT = 2.f;
CHAOS_VAR float angleLR;
CHAOS_VAR float angleUD;

CHAOS_VAR Camera leanedCamera = 0;

struct Message
{
	ChatMessage m_Message;
	int m_ReceivedTimestamp;
	enum Position
	{
		LEFT,
		RIGHT,
		UP,
		DOWN
	} m_Position;

	Message(ChatMessage message, int timestamp, Position position)
	    : m_Message(message), m_ReceivedTimestamp(timestamp), m_Position(position)
	{
	}
};

CHAOS_VAR std::list<Message> messageQueue;
CHAOS_VAR std::map<std::string, std::string> usercolors;

static void ProcessMessage(const ChatMessage &message)
{
	std::string m = message.m_Message;

	RemoveSpaces(m);
	if (!CompareCaseInsensitive(m, "LEFT"))
	{
		angleLR += ANGLE_INCREMENT;
		messageQueue.push_back({ message, GET_GAME_TIMER(), Message::LEFT });
	}
	if (!CompareCaseInsensitive(m, "RIGHT"))
	{
		angleLR -= ANGLE_INCREMENT;
		messageQueue.push_back({ message, GET_GAME_TIMER(), Message::RIGHT });
	}
	if (!CompareCaseInsensitive(m, "UP"))
	{
		angleUD += ANGLE_INCREMENT;
		messageQueue.push_back({ message, GET_GAME_TIMER(), Message::UP });
	}
	if (!CompareCaseInsensitive(m, "DOWN"))
	{
		angleUD -= ANGLE_INCREMENT;
		messageQueue.push_back({ message, GET_GAME_TIMER(), Message::DOWN });
	}
}

static void OnStart()
{
	angleLR = angleUD = 0;

	if (ComponentExists<Voting>())
	{
		m_OnNewMessageListener.Register(GetComponent<Voting>()->OnNewMessage,
		                                [&](const ChatMessage &message) { ProcessMessage(message); });
	}

	leanedCamera = CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", 1);
	RENDER_SCRIPT_CAMS(true, true, 700, 1, 1, 1);
}

static void OnTick()
{
	SET_CAM_ACTIVE(leanedCamera, true);
	Vector3 coord = GET_GAMEPLAY_CAM_COORD();
	Vector3 rot   = GET_GAMEPLAY_CAM_ROT(2);
	float fov     = GET_GAMEPLAY_CAM_FOV();
	SET_CAM_PARAMS(leanedCamera, coord.x, coord.y, coord.z, angleUD, angleLR, rot.z, fov, 0, 1, 1, 2);

	DrawScreenText("Chat, type LEFT, RIGHT, UP or DOWN~n~to move streamer's camera!", { 0.8f, 0.8f }, 0.6f,
	               { 255, 255, 255 }, true);

	for (auto it = messageQueue.begin(); it != messageQueue.end();)
	{
		const auto message = *it;

		float x = 0.5f, y = 0.55f;

		if (message.m_Position == message.LEFT)
			x = 0.05f;
		else if (message.m_Position == message.RIGHT)
			x = 0.95f;
		else if (message.m_Position == message.UP)
			y = 0.05f;
		else if (message.m_Position == message.DOWN)
			y = 0.95f;

		y += -(GET_GAME_TIMER() - message.m_ReceivedTimestamp) / 1000.f * 0.1f;

		auto userid = message.m_Message.m_Userstate.m_Userid;
		auto color  = message.m_Message.m_Userstate.m_ColorHex;
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

		auto username      = message.m_Message.m_Userstate.m_Username;
		auto displayName   = message.m_Message.m_Userstate.m_DisplayName;

		// hack - should've checked for displayName.toLower() != username, but I'm too lazy
		if (displayName.size() != username.size())
			displayName = username;

		DrawScreenText(displayName, { x, y }, 0.4f, Color(color), true);

		if (GET_GAME_TIMER() - message.m_ReceivedTimestamp > 2000)
			it = messageQueue.erase(it);
		else
			it++;
	}
}

static void OnStop()
{
	SET_CAM_ACTIVE(leanedCamera, false);
	RENDER_SCRIPT_CAMS(false, true, 700, 1, 1, 1);
	DESTROY_CAM(leanedCamera, true);
	leanedCamera = 0;

	if (ComponentExists<Voting>())
		m_OnNewMessageListener.Unregister(GetComponent<Voting>()->OnNewMessage);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick,
	{
		.Name = "Chat Controls Camera",
		.Id = "screen_chat_control_camera",
		.IsTimed = true,
		.ConditionType = EffectConditionType::SilentVotingEnabled
	}
);