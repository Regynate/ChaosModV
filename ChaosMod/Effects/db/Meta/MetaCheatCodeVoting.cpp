#include <stdafx.h>

#include <queue>

#include "Memory/Hooks/GetLabelTextHook.h"

#include "Components/EffectDispatcher.h"
#include "Components/SplashTexts.h"
#include "Components/Voting.h"
#include "Effects/EnabledEffects.h"
#include "Util/Chat/ChatMessage.h"
#include "Util/Events.h"
#include "Util/Random.h"
#include "Util/Text.h"

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR CHAOS_EVENT_LISTENER(Voting::OnNewMessage) m_OnNewMessageListener;

CHAOS_VAR std::list<ChatMessage> messageQueue;

struct EffectEntry
{
	std::string Id;
	std::string Name;
	std::string NameNoSpaces;
};

CHAOS_VAR std::list<EffectEntry> availableEffects;

struct EffectWithAuthor
{
	EffectIdentifier Id;
	std::string Author;
};

CHAOS_VAR std::list<EffectWithAuthor> effectQueue;
CHAOS_VAR size_t startTick;

CHAOS_VAR const int VOTING_TIME = 25000;

static void RemoveSpaces(std::string &data)
{
	std::string buffer;
	buffer.reserve(data.size());
	for (size_t pos = 0; pos != data.size(); ++pos)
		if ((data[pos] > 'a' && data[pos] < 'z') || (data[pos] > 'A' && data[pos] < 'Z'))
			buffer.append(&data[pos], 1);
	data.swap(buffer);
}

static void AddMessageToQueue(ChatMessage message)
{
	for (const auto &effect : GetFilteredEnabledEffects())
		if (!effect->IsExcludedFromCheatVoting())
		{
			const auto name   = effect->HasCustomName() ? effect->CustomName : effect->Name;
			auto nameNoSpaces = name;
			RemoveSpaces(nameNoSpaces);

			availableEffects.emplace_back(effect->Id, name, nameNoSpaces);
		}

	messageQueue.push_back(message);
}

static void OnStart()
{
	if (ComponentExists<Voting>())
	{
		m_OnNewMessageListener.Register(GetComponent<Voting>()->OnNewMessage,
		                                [&](const ChatMessage &message) { AddMessageToQueue(message); });
	}

	if (ComponentExists<SplashTexts>())
		GetComponent<SplashTexts>()->ShowSplash(
		    "CHEAT CODE VOTING~n~Type an effect name~n~(with or without spaces, I don't care)~n~into chat!",
		    { 0.5f, 0.3f }, 1.2f, { 255, 255, 255 }, VOTING_TIME / 1000.f);

	startTick = GetTickCount64();
}

static void OnTick()
{
	if (GetTickCount64() - startTick < VOTING_TIME)
	{
		while (!messageQueue.empty())
		{
			auto message = messageQueue.front();
			messageQueue.pop_front();

			for (const auto &entry : availableEffects)
			{
				RemoveSpaces(message.m_Message);
				if (!CompareCaseInsensitive(message.m_Message, entry.NameNoSpaces))
				{
					auto username    = message.m_Userstate.m_Username;
					auto displayName = message.m_Userstate.m_DisplayName;

					if (displayName.size() != username.size())
						displayName = username;

					effectQueue.push_back({ entry.Id, displayName });
					break;
				}
			}
		}
	}
	else
	{
		float percentage = CurrentEffect::GetEffectCompletionPercentage();
		int waitTime     = effectQueue.empty()
		                     ? 0
		                     : (int)std::max((VOTING_TIME / percentage * (1.f - percentage)) / effectQueue.size(), 0.f);

		while (!effectQueue.empty())
		{
			auto entry = effectQueue.front();
			effectQueue.pop_front();

			if (ComponentExists<EffectDispatcher>())
				GetComponent<EffectDispatcher>()->DispatchEffect(entry.Id, {}, "(" + entry.Author + ")");

			WAIT(waitTime);
		}
	}
}

static void OnStop()
{
	if (ComponentExists<Voting>())
		m_OnNewMessageListener.Unregister(GetComponent<Voting>()->OnNewMessage);

	messageQueue.clear();
	availableEffects.clear();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick,
	{
		.Name = "Cheat Code Voting",
		.Id = "meta_cheat_code_voting",
		.IsTimed = true,
        .IsShortDuration = true,
        .ExecutionType = EffectExecutionType::Meta,
		.ConditionType = EffectConditionType::SilentVotingEnabled
	}
);