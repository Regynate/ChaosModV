#pragma once
#include "Component.h"


class ZChaosManager : public Component
{
  private:
	struct ZChaosEffect
	{
		int time;
		int defaultTime;
		void (*m_OnStart)(ZChaosEffect *);
		void (*m_OnTick)(ZChaosEffect *);
		char *name;
		char *name2;
		int field_28;
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
	bool (*IsEffectEnabled)(ZChaosEffect* effect, bool a1);

	void CheckAndAddEffects();

  protected:
	ZChaosManager();
	virtual ~ZChaosManager() override;

  public:
	virtual void OnRun() override;
	virtual void OnModPauseCleanup() override;
	void RunEffectsOnTick(int a);

	template <class T>
	requires std::is_base_of_v<Component, T>
	friend struct ComponentHolder;
};
