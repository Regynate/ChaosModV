#pragma once

#include <list>

#include "Component.h"
#include "Components/EffectDispatcher.h"
#include "Memory/Hooks/ScriptThreadRunHook.h"
#include "Util/OptionsFile.h"

class PermanentChallenge : public Component
{
  private:
	struct MissionData
	{
		bool completed;
		char _pad1[4];

		int missionFailsNoProgress;
		char _pad2[4];

		int missionFailsTotal;
		char _pad3[4];

		int completionOrder;
		char _pad4[4];

		int score;
		char _pad5[4];

		float statCompletion;
		char _pad6[4];
	};

	struct MissionArray
	{
		int size;
		char _pad[4];

		MissionData data[100];
	};

	CHAOS_EVENT_LISTENER(Hooks::OnScriptThreadRun) m_SearchMissionGlobalListener;
	CHAOS_EVENT_LISTENER(EffectDispatcher::OnClearedEffects) m_ClearEffectsListener;

	OptionsFile m_ConfigFile { { "chaosmod/configs/cached/permanentchallenge.json" } };

	bool m_Enabled         = false;

	int m_MaxEffects       = 10;
	int m_LastMissionCount = 0;

	std::vector<std::string> m_Effects;

	bool m_SearchedForMissionStateGlobal = 0;
	int m_MissionStateGlobalIdx          = 0;
	int m_RCMissionStateGlobalIdx        = 0;

	bool m_WaitingConfirmReroll          = false;
	bool m_RerollEffect                  = false;

	bool m_WaitingStart                  = false;
	bool m_DoStart                       = false;

	bool SearchMissionStateGlobal(rage::scrThread *thread);

	int GetMissionsCount();

	std::vector<std::string> GetValidEffects();
	void RevalidateEffects();
	void InitEffects();
	void RerunEffects();
	void RerunEffectsNoDispatch();

	inline int MinEffectIndex()
	{
		return std::max(m_LastMissionCount - m_MaxEffects, 0);
	}
	inline int MaxEffectIndex()
	{
		return std::min(m_LastMissionCount, (int)m_Effects.size());
	}

  public:
	PermanentChallenge();

	inline bool IsEnabled()
	{
		return m_Enabled;
	}

	virtual void OnRun() override;
	virtual void OnModPauseCleanup() override;
	virtual void OnKeyInput(DWORD key, bool repeated, bool isUpNow, bool isCtrlPressed, bool isShiftPressed,
	                        bool isAltPressed) override;
};