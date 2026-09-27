#pragma once

#include "Components/Component.h"

#include "Effects/EffectData.h"
#include "Effects/EffectIdentifier.h"
#include "Effects/EffectThreads.h"
#include "Effects/Register/RegisteredEffects.h"

#include "Util/Events.h"

#include <array>
#include <cstdint>
#include <list>
#include <queue>
#include <string_view>
#include <vector>

class EffectDispatcher : public Component
{
  public:
	enum DispatchEffectFlags
	{
		DispatchEffectFlag_None,
		// Whether this effect should not be recorded in the effect replay log (used for e.g. the "Re-Invoke Previous
		// Effects" meta effect)
		DispatchEffectFlag_NoAddToLog = (1 << 0),
	};
	struct EffectDispatchEntry
	{
		EffectIdentifier Id;
		bool IsPermanent;
		bool IsPending;
		std::string Suffix;
		DispatchEffectFlags Flags;
		std::string Context;
	};
	std::queue<EffectDispatchEntry> EffectDispatchQueue;

	struct ActiveEffect
	{
		EffectIdentifier Id;

		std::string Name;
		std::string FakeName;

		RegisteredEffect *RegisteredEffect = nullptr;
		LPVOID ThreadId                    = nullptr;

		float Timer                        = 0.f;
		float MaxTime                      = 0.f;
		bool IsTimed                       = false;
		bool IsPermanent                   = false;
		bool IsPending                     = false;

		bool IsMeta                        = false;

		bool HideEffectName                = false;
		bool IsStopping                    = false;
		bool IsZombie                      = false;

		DWORD64 SoundId                    = 0;

		void Stop();
	};
	struct
	{
		std::vector<ActiveEffect> PendingEffects;
		std::vector<ActiveEffect> ActiveEffects;
		std::vector<ActiveEffect> PermanentEffects;
		std::list<RegisteredEffect *> DispatchedEffectsLog;
		float MetaEffectTimerPercentage   = 0.f;
		float MetaEffectSpawnTime         = 0;
		float MetaEffectTimedDur          = 0;
		float MetaEffectShortDur          = 0;
		float EffectTimedDur              = 0;
		float EffectTimedShortDur         = 0;
		float PermanentNonTimedRestartDur = 0;
		bool MetaEffectsEnabled           = true;
	} SharedState;

  public:
	enum class ClearEffectsState
	{
		None,
		All,
		AllRestartPermanent
	} m_ClearEffectsState = ClearEffectsState::None;

	ChaosCancellableEvent<const EffectIdentifier &> OnPreDispatchEffect;
	ChaosEvent<const EffectIdentifier &, const std::string> OnDispatchEffectFailed;
	ChaosEvent<const EffectIdentifier &, const std::string> OnPostDispatchEffect;

	ChaosEvent<const EffectIdentifier &> OnPreRunEffect;
	ChaosEvent<const EffectIdentifier &> OnPostRunEffect;
	ChaosEvent<const ClearEffectsState &> OnClearedEffects;

  private:
	int m_MaxRunningEffects = 0;

	Color m_TextColor;
	Color m_EffectTimerColor;

	std::string m_LastEffect;

	bool m_DisableDrawEffectTexts     = false;

	bool m_EnableNormalEffectDispatch = false;

  public:
	bool EnableEffectTextExtraTopSpace = false;

	EffectDispatcher();

  private:
	float GetEffectTopSpace();

	void RegisterPermanentEffects();

  public:
	void DispatchPermanentEffect(const EffectIdentifier &effectId);
	void DispatchPendingEffect(const EffectIdentifier &effectId);

	virtual void OnModPauseCleanup() override;
	virtual void OnRun() override;

	void DrawEffectTexts();

	void DispatchEffect(const EffectIdentifier &effectId,
	                    DispatchEffectFlags dispatchEffectFlags = DispatchEffectFlag_None,
	                    const std::string &suffix = {}, const std::string &context = {});

	std::string GetRandomEffectId() const;
	void DispatchRandomEffect(DispatchEffectFlags dispatchEffectFlags = DispatchEffectFlag_None,
	                          const std::string &suffix = {}, const std::string &context = {});

	void CheckClearState();
	void UpdateEffects(float deltaTime);
	void UpdateMetaEffects(float deltaTime);

	void ClearEffect(const EffectIdentifier &effectId);
	enum ClearEffectsFlags
	{
		ClearEffectsFlag_None,
		// Whether permanent effects should not be started again after clearing all effects
		ClearEffectsFlag_NoRestartPermanentEffects = (1 << 0),
	};
	void ClearEffects(ClearEffectsFlags clearEffectFlags = ClearEffectsFlag_None);
	void ClearActiveEffects(const std::string &ignoreEffect = {});
	void ClearMostRecentEffect();

	float GetRemainingTimeForEffect(const EffectIdentifier &effectId);
	void SetRemainingTimeForEffect(const EffectIdentifier &effectId, float value);

	std::vector<RegisteredEffect *> GetRecentEffects(int distance, const std::string &ignoreEffect = {}) const;

	void Reset(ClearEffectsFlags clearEffectFlags = ClearEffectsFlag_None);

	bool IsClearingEffects() const;

	EffectIdentifier GetLastEffectId() const;
};
