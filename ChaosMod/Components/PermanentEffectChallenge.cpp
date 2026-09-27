#include <stdafx.h>

#include <algorithm>

#include "Components/EffectDispatchTimer.h"
#include "Components/PermanentEffectChallenge.h"
#include "Effects/EnabledEffects.h"
#include "Lib/scrThread.h"
#include "Main.h"
#include "Memory/Hooks/GetLabelTextHook.h"
#include "Memory/Script.h"
#include "Memory/WeaponPool.h"
#include "Util/HelpText.h"
#include "Util/OptionsManager.h"

bool PermanentChallenge::SearchMissionStateGlobal(rage::scrThread *thread)
{
	if (!m_SearchedForMissionStateGlobal && !m_MissionStateGlobalIdx
	    && !strcmp(thread->GetName(), "CompletionPercentage_controller"))
	{
		auto program = Memory::ScriptThreadToProgram(thread);
		if (program && program->m_CodeBlocks)
		{
			m_SearchedForMissionStateGlobal = true;

			Handle handle                   = Memory::FindScriptPattern("2D ?? ?? ?? ?? 71 39 ?? 71", program);

			if (!handle.IsValid())
			{
				LOG("Mission index global not found; Mission can not be enabled!");
			}
			else
			{
				m_MissionStateGlobalIdx = handle.At(10).Value<int>() & 0xFFFFFF;
				LOG("Mission index global found (Global: " << m_MissionStateGlobalIdx << ")");
				LOG("Address: " << Memory::GetGlobalPtr(m_MissionStateGlobalIdx));

				m_RCMissionStateGlobalIdx = handle.At(25).Value<int>() & 0xFFFFFF;
				LOG("RC Mission index global found (Global: " << m_RCMissionStateGlobalIdx << ")");
				LOG("Address: " << Memory::GetGlobalPtr(m_RCMissionStateGlobalIdx));
			}
		}
	}

	return true;
}

PermanentChallenge::PermanentChallenge() : Component()
{
	m_Enabled    = g_OptionsManager.GetConfigValue("EnablePermanentChallenge", false);
	m_MaxEffects = g_OptionsManager.GetConfigValue("PermanentChallengeMaxEffects", 10);

	LOG("Permanent challenge enabled: " << (m_Enabled ? "true" : "false"));

	if (!m_Enabled)
		return;

	m_Effects = m_ConfigFile.ReadValue("effects", std::vector<std::string>());

	if (m_Effects.size() == 0)
		InitEffects();
	else
		RevalidateEffects();

	m_WaitingStart = true;

	RerunEffectsNoDispatch();

	m_SearchMissionGlobalListener.Register(Hooks::OnScriptThreadRun, [this](rage::scrThread *thread)
	                                       { return SearchMissionStateGlobal(thread); });

	if (ComponentExists<EffectDispatchTimer>())
		GetComponent<EffectDispatchTimer>()->SetTimerEnabled(false);

	if (ComponentExists<EffectDispatcher>())
		m_ClearEffectsListener.Register(GetComponent<EffectDispatcher>()->OnClearedEffects,
		                                [&](const EffectDispatcher::ClearEffectsState &state)
		                                {
			                                if (state == EffectDispatcher::ClearEffectsState::AllRestartPermanent)
				                                RerunEffects();
		                                });
}

static std::vector<std::string> &Shuffle(std::vector<std::string> &arr)
{
	for (int i = arr.size() - 1; i > 0; --i)
	{
		int j  = g_Random.GetRandomInt(0, i);
		auto t = arr[j];
		arr[j] = arr[i];
		arr[i] = t;
	}

	return arr;
}

std::vector<std::string> PermanentChallenge::GetValidEffects()
{
	std::vector<std::string> effects;

	for (const auto &effect : GetFilteredEnabledEffects())
		if (!effect->IsMeta() /* && (effect->TimedType != EffectTimedType::NotTimed) */)
			effects.push_back(effect->Id);
		else
			LOG(effect->Name);

	if (effects.size() < 69 + 20)
	{
		std::string message = "Not enough enabled effects for permanent mission challenge!";

		std::wstring wStr   = { message.begin(), message.end() };
		MessageBox(NULL, wStr.c_str(), L"ChaosModV Error", MB_OK | MB_ICONERROR);

		return {};
	}

	return effects;
}

void PermanentChallenge::InitEffects()
{
	std::vector<std::string> effects = GetValidEffects();

	Shuffle(effects);

	m_Effects.clear();

	if (effects.size() > 0)
		for (int i = 0; i < 69 + 20; ++i)
			m_Effects.push_back(effects[i]);

	m_ConfigFile.SetValue("effects", m_Effects);
	m_ConfigFile.WriteFile();
}

void PermanentChallenge::RevalidateEffects()
{
	std::vector<std::string> effects = GetValidEffects();

	Shuffle(effects);

	if (effects.size() > 0)
		for (int i = 0; i < m_Effects.size(); ++i)
		{
			// override m_Effects[i] if it's not valid (not in valid effects vector)
			if (std::ranges::find(effects, m_Effects[i]) == std::ranges::end(effects))
			{
				LOG("Effect " << m_Effects[i] << " not in valid effects; rerolling to " << effects[0]);
				m_Effects[i] = effects[0];
			}

			// ugly but works ig
			// remove current effect to ensure no duplicates
			std::erase(effects, m_Effects[i]);
		}

	m_ConfigFile.SetValue("effects", m_Effects);
	m_ConfigFile.WriteFile();
}

int PermanentChallenge::GetMissionsCount()
{
	if (!m_MissionStateGlobalIdx || !m_RCMissionStateGlobalIdx)
		return -1;

	int missionCount   = *reinterpret_cast<int *>(Memory::GetGlobalPtr(m_MissionStateGlobalIdx));
	int rcMissionCount = *reinterpret_cast<int *>(Memory::GetGlobalPtr(m_RCMissionStateGlobalIdx));

	return missionCount + rcMissionCount;
}

void PermanentChallenge::OnRun()
{
	if (!m_Enabled)
		return;

	if (m_WaitingConfirmReroll)
		DisplayHelpText("Press Enter to confirm reroll last effect");
	else if (m_WaitingStart)
		DisplayHelpText("Press Ctrl+I to start the challenge. Press Ctrl+O to reroll last effect");

	int missionCount    = GetMissionsCount();
	bool reapplyEffects = false;

	if (m_DoStart)
	{
		reapplyEffects = true;
		m_DoStart      = false;
	}

	int maxEffectIndex = MaxEffectIndex();

	if (m_RerollEffect && m_Effects.size() > 0)
	{
		m_RerollEffect = false;
		// yay more ugly
		LOG("Rerolling effect " << m_Effects[maxEffectIndex - 1]);
		m_Effects[maxEffectIndex - 1] = "";
		RevalidateEffects();
		reapplyEffects = true;
	}

	if (missionCount != m_LastMissionCount || reapplyEffects)
	{
		m_LastMissionCount = missionCount;
		if (m_WaitingStart)
			RerunEffectsNoDispatch();
		else
			RerunEffects();
	}
}

void PermanentChallenge::RerunEffects()
{
	if (m_WaitingStart)
		return;

	if (ComponentExists<EffectDispatcher>())
	{
		GetComponent<EffectDispatcher>()->ClearEffects(EffectDispatcher::ClearEffectsFlag_NoRestartPermanentEffects);

		for (int i = MinEffectIndex(); i < MaxEffectIndex(); ++i)
		{
			// eww O(n*m)
			for (auto &[effectId, effectData] : g_EnabledEffects)
				if (effectId.Id() == m_Effects[i])
					GetComponent<EffectDispatcher>()->DispatchPermanentEffect(effectId);
		}
	}
}

void PermanentChallenge::RerunEffectsNoDispatch()
{
	if (!m_WaitingStart)
		return;

	if (ComponentExists<EffectDispatcher>())
	{
		GetComponent<EffectDispatcher>()->ClearEffects(EffectDispatcher::ClearEffectsFlag_NoRestartPermanentEffects);

		for (int i = MinEffectIndex(); i < MaxEffectIndex(); ++i)
		{
			// eww O(n*m)
			for (auto &[effectId, effectData] : g_EnabledEffects)
				if (effectId.Id() == m_Effects[i])
					GetComponent<EffectDispatcher>()->DispatchPendingEffect(effectId);
		}
	}
}

void PermanentChallenge::OnModPauseCleanup()
{
}

void PermanentChallenge::OnKeyInput(DWORD key, bool repeated, bool isUpNow, bool isCtrlPressed, bool isShiftPressed,
                                    bool isAltPressed)
{
	if (!repeated)
	{
		if (isCtrlPressed && key == 0x4F) // O
			m_WaitingConfirmReroll = true;
		if (m_WaitingConfirmReroll && key == 0x0D) // Enter
		{
			m_WaitingConfirmReroll = false;
			m_RerollEffect         = true;
		}
		if (m_WaitingStart && isCtrlPressed && key == 0x49) // I
		{
			m_WaitingStart = false;
			m_DoStart      = true;
		}
	}

	if (key != 0x4F && key != 0x11 && key != 0xA2 && key != 0xA3)
		m_WaitingConfirmReroll = false;
}