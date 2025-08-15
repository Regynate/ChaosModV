#include <stdafx.h>

#include <queue>

#include "Memory/Hooks/GetLabelTextHook.h"

#include "Components/EntityTracking.h"
#include "Components/Voting.h"
#include "Util/Chat/ChatMessage.h"
#include "Util/Events.h"
#include "Util/HelpText.h"
#include "Util/Peds.h"
#include "Util/Random.h"
#include "Util/ScriptText.h"

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR CHAOS_EVENT_LISTENER(Voting::OnNewMessage) m_OnNewMessageListener;
CHAOS_VAR CHAOS_EVENT_LISTENER(Voting::OnMessageDelete) m_OnMessageDeleteListener;
CHAOS_VAR CHAOS_EVENT_LISTENER(Voting::OnUserBan) m_OnUserBanListener;

struct User
{
	std::string m_Userid;
	ChatMessage m_LastMessage;
	int m_LastMessageTimestamp;

	User(ChatMessage message, int timestamp)
	    : m_Userid(message.m_Userstate.m_Userid), m_LastMessage(message), m_LastMessageTimestamp(timestamp)
	{
	}
};

struct SpawnedUser
{
	User m_User;
	Ped m_Ped;

	SpawnedUser(User user, Ped ped) : m_User(user), m_Ped(ped)
	{
	}
};

CHAOS_VAR std::list<User> userQueue;
CHAOS_VAR std::map<std::string, SpawnedUser> spawnedUsers;

CHAOS_VAR std::map<std::string, std::string> usercolors;

CHAOS_VAR std::mutex messageMutex;

CHAOS_VAR const int MAX_USERS = 10;

static void encode(std::string &data)
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

static void RemovePed(Ped ped)
{
	if (!DOES_ENTITY_EXIST(ped))
		return;

	REQUEST_NAMED_PTFX_ASSET("scr_sr_tr");
	while (!HAS_NAMED_PTFX_ASSET_LOADED("scr_sr_tr"))
		WAIT(0);
	Vector3 pedPos = GET_ENTITY_COORDS(ped, false);
	USE_PARTICLE_FX_ASSET("scr_sr_tr");
	START_PARTICLE_FX_NON_LOOPED_AT_COORD("scr_sr_tr_car_change", pedPos.x, pedPos.y, pedPos.z, 0, 0, 0, 1, false, true,
	                                      false);

	PLAY_SOUND_FROM_COORD(-1, "BBQ_EXPLODE", pedPos.x, pedPos.y, pedPos.z, "JOSH_03_SOUNDSET", 1, 0, 0);
	WAIT(200);
	DELETE_ENTITY(&ped);
}

static void AddMessageToQueue(ChatMessage message)
{
	std::lock_guard lock(messageMutex);

	auto m = message.m_Message;
	if (m.size() == 1 && m[0] >= '1' && m[0] <= '9')
		return;

	const auto &userid = message.m_Userstate.m_Userid;

	if (spawnedUsers.contains(userid))
	{
		spawnedUsers.at(userid).m_User.m_LastMessage          = message;
		spawnedUsers.at(userid).m_User.m_LastMessageTimestamp = GetTickCount64();
	}

	for (auto it = userQueue.begin(); it != userQueue.end();)
		if (it->m_Userid == userid)
			it = userQueue.erase(it);
		else
			++it;

	while (userQueue.size() > MAX_USERS)
		userQueue.pop_front();

	userQueue.push_back(User(message, GetTickCount64()));
}

static void RemoveMessageById(std::string id)
{
	std::lock_guard lock(messageMutex);

	userQueue.remove_if([id](User u) { return u.m_LastMessage.m_Id == id; });

	for (auto &[_, user] : spawnedUsers)
		if (user.m_User.m_LastMessage.m_Id == id)
		{
			user.m_User.m_LastMessage.m_Message = "";
			user.m_User.m_LastMessageTimestamp  = 0;
		}
}

static void RemoveMessagesByUserId(std::string userid)
{
	std::lock_guard lock(messageMutex);

	userQueue.remove_if([userid](User u) { return u.m_Userid == userid; });

	if (spawnedUsers.contains(userid))
		spawnedUsers.erase(userid);
}

static void UpdateUser(Ped ped, std::string userid)
{
	if (!spawnedUsers.contains(userid))
		return;

	auto user    = spawnedUsers.at(userid).m_User;

	auto message = user.m_LastMessage;
	auto m       = message.m_Message;
	encode(m);

	auto color = message.m_Userstate.m_ColorHex;
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

	auto username      = message.m_Userstate.m_Username;
	auto displayName   = message.m_Userstate.m_DisplayName;

	// hack - should've checked for displayName.toLower() != username, but I'm too lazy
	if (displayName.size() != username.size())
		displayName = username;

	float nameDistance = 0.4f;

	if (GetTickCount64() - user.m_LastMessageTimestamp < 5000)
	{
		DrawTextAbovePedHead(user.m_LastMessage.m_Message, ped, 1.f, Color(255, 255, 255), 0.4f, 0.3f);
		nameDistance = 0.6f;
	}
	DrawTextAbovePedHead(displayName, ped, 2.f, Color(color), nameDistance);
}

static Ped SpawnEnemy()
{
	static const Hash playerGroup = "PLAYER"_hash;
	static const Hash civGroup    = "CIVMALE"_hash;
	static const Hash femCivGroup = "CIVFEMALE"_hash;

	Hash relationshipGroup;
	ADD_RELATIONSHIP_GROUP("_HOSTILE_RANDOM", &relationshipGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(5, relationshipGroup, playerGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(5, relationshipGroup, civGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(5, relationshipGroup, femCivGroup);

	Ped playerPed     = PLAYER_PED_ID();
	Vector3 playerPos = GET_ENTITY_COORDS(playerPed, false);

	Ped ped           = CreateRandomPoolPed(playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(playerPed));
	if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
		SET_PED_INTO_VEHICLE(ped, GET_VEHICLE_PED_IS_IN(playerPed, false), -2);

	SET_PED_RELATIONSHIP_GROUP_HASH(ped, relationshipGroup);
	SET_PED_HEARING_RANGE(ped, 9999.f);
	SET_PED_CONFIG_FLAG(ped, 281, true);

	SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(ped, false);
	SET_RAGDOLL_BLOCKING_FLAGS(ped, 5);
	SET_PED_SUFFERS_CRITICAL_HITS(ped, false);

	SET_PED_COMBAT_ATTRIBUTES(ped, 5, true);
	SET_PED_COMBAT_ATTRIBUTES(ped, 46, true);

	GIVE_WEAPON_TO_PED(ped, GET_SELECTED_PED_WEAPON(playerPed), 9999, true, true);

	SET_PED_ACCURACY(ped, 100);
	SET_PED_FIRING_PATTERN(ped, 0xC6EE6B4C);

	return ped;
}

static Ped SpawnCompanion()
{
	Ped playerPed     = PLAYER_PED_ID();
	Vector3 playerPos = GET_ENTITY_COORDS(playerPed, false);

	Ped ped           = CreateRandomPoolPed(playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(playerPed));
	if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
		SET_PED_INTO_VEHICLE(ped, GET_VEHICLE_PED_IS_IN(playerPed, false), -2);

	SET_PED_SUFFERS_CRITICAL_HITS(ped, false);

	SetCompanionRelationship(ped, "_COMPANION_RANDOM");
	SET_PED_HEARING_RANGE(ped, 9999.f);

	SET_PED_AS_GROUP_MEMBER(ped, GET_PLAYER_GROUP(PLAYER_ID()));

	GIVE_WEAPON_TO_PED(ped, GET_SELECTED_PED_WEAPON(playerPed), 9999, true, true);

	SET_PED_ACCURACY(ped, 100);
	SET_PED_FIRING_PATTERN(ped, 0xC6EE6B4C);

	return ped;
}

static SpawnedUser SpawnUser(const User user)
{
	const auto ped    = g_Random.GetRandomInt(0, 3) == 0 ? SpawnEnemy() : SpawnCompanion();

	const auto userid = user.m_Userid;

	if (ComponentExists<Tracking>())
	{
		GetComponent<Tracking>()->AddEntityTracker(ped,
		                                           [userid](Ped ped)
		                                           {
			                                           UpdateUser(ped, userid);

			                                           if (!spawnedUsers.contains(userid))
			                                           {
				                                           RemovePed(ped);
				                                           return false;
			                                           }

			                                           return true;
		                                           });
	}

	return { user, ped };
}

static void SpawnUsers()
{
	std::lock_guard lock(messageMutex);

	for (const auto &user : userQueue)
		spawnedUsers.emplace(user.m_Userid, SpawnUser(user));
}

static void Cleanup()
{
	std::lock_guard lock(messageMutex);

	if (ComponentExists<Voting>())
	{
		m_OnNewMessageListener.Unregister(GetComponent<Voting>()->OnNewMessage);
		m_OnMessageDeleteListener.Unregister(GetComponent<Voting>()->OnMessageDelete);
		m_OnUserBanListener.Unregister(GetComponent<Voting>()->OnUserBan);
	}

	userQueue.clear();
	usercolors.clear();
}

static void OnStart()
{
	SpawnUsers();
}

static void OnInit()
{
	Cleanup();

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
	else
	{
		LOG("VOTING COMPONENT NOT INITIALIZED");
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, OnInit,
	{
		.Name = "Spawn Twitch Chatters",
		.Id = "misc_spawn_twitch_chatters",
		.ConditionType = EffectConditionType::SilentVotingEnabled
	}
);