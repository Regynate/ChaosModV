#pragma once
#include "Component.h"

#include "Effects/EffectData.h"
#include "Effects/EffectConfig.h"
#include "Effects/EnabledEffects.h"

class ZChaosManager : public Component
{
  public:
	struct ZChaosEffect
	{
		int time;
		int defaultTime;
		void (*m_OnStart)(ZChaosEffect *);
		void (*m_OnTick)(ZChaosEffect *);
		char *name;
		char *name2;
		int cycleType;
		char __pad1[12];
		bool enabledManually;
		bool enabledForCheatCodeVoting;
		bool enabledForSubEffects;
		bool enabled1;
		bool enabled2;
		bool cannotBeAutoDisabled;
		char __pad2[2];
		__int64 flags;
		char __pad3[8];
		char *disableReason;
		char *disableReason2;
		bool field_60;
		char __pad4[15];
		int triggerCount;
		char __pad5[36];
		ZChaosEffect *effectPtr;
		double cutoutTime;
	};

  private:
	struct ZChaosEffectsList
	{
		ZChaosEffect **first;
		ZChaosEffect **last;
	};

	int m_EffectCount;
	ZChaosEffect **m_EffectsArray;
	std::map<std::string, ZChaosEffect> m_ActiveZChaosEffects;
	std::unordered_map<EffectIdentifier, EffectData, EffectsIdentifierHasher> m_EnabledEffects;
	char *m_NoTimer;
	char *m_DisableChaosUI;
	char *m_ZChaosActive;
	double *m_WarningTime;
	char *m_WarningStr;
	char *m_WarningStr2;
	bool (*IsEffectEnabled)(ZChaosEffect *effect, bool a1);

	void AddEffects();
	void OverrideWarning();
	EffectData GetDefaultEffectData(ZChaosEffect *effect, std::string effectId);

  public:
	ZChaosManager();

	virtual void OnRun() override;
	virtual void OnModPauseCleanup() override;
	void RunEffectsOnTick(int a);
	std::string GetIdForEffect(ZChaosEffect *effect);
	void RegisterZChaosEffect(ZChaosEffect *effect, std::string effectId);
	void EnableEffect(std::string effectId, EffectData effectData);

	template <class T>
	requires std::is_base_of_v<Component, T>
	friend struct ComponentHolder;
};