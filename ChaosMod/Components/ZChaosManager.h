#pragma once
#include "Component.h"


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

	int m_iEffectCount;
	ZChaosEffect **m_rgEffectsArray;
	std::map<std::string, ZChaosEffect> m_rgActiveZChaosEffects;
	char *m_pNoTimer;
	char *m_pDisableChaosUI;
	char *m_pZChaosActive;
	double *m_pWarningTime;
	char *m_pWarningStr;
	char *m_pWarningStr2;
	bool (*IsEffectEnabled)(ZChaosEffect *effect, bool a1);


	void CheckAndAddEffects();
	std::string GetIdForEffect(ZChaosEffect *effect, int index);
	void OverrideWarning();

  protected:
	ZChaosManager();
	virtual ~ZChaosManager() override;

  public:
	virtual void OnRun() override;
	virtual void OnModPauseCleanup() override;
	void RunEffectsOnTick(int a);
	std::string GetIdForEffect(ZChaosEffect *effect);
	void RegisterZChaosEffect(ZChaosEffect *effect, std::string effectId);
	void EnableEffect(ZChaosManager::ZChaosEffect *effect, std::string effectId);

	template <class T>
	requires std::is_base_of_v<Component, T>
	friend struct ComponentHolder;
};
