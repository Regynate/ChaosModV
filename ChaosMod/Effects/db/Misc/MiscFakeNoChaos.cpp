#include <stdafx.h>

#include "Components/EffectDispatcher.h"
#include "Components/MetaModifiers.h"
#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR std::list<Entity> m_SpawnedEntities;
CHAOS_VAR std::vector<EffectDispatcher::ActiveEffect> m_RunningEffects;

static void OnStart()
{
	m_SpawnedEntities = GetPoolEntities();

	for (const auto &ped : GetAllPeds())
	{
		if (IS_PED_IN_ANY_VEHICLE(ped, true))
		{
			auto vehicle = GET_VEHICLE_PED_IS_IN(ped, true);
			if (std::find(m_SpawnedEntities.begin(), m_SpawnedEntities.end(), vehicle) != m_SpawnedEntities.end()
			    && std::find(m_SpawnedEntities.begin(), m_SpawnedEntities.end(), ped) == m_SpawnedEntities.end())
			{
				TASK_LEAVE_VEHICLE(ped, vehicle, 16);
			}
		}
	}

	for (const auto &entity : m_SpawnedEntities)
	{
		FREEZE_ENTITY_POSITION(entity, true);
		SET_ENTITY_ALPHA(entity, 0, false);
		SET_ENTITY_COLLISION(entity, false, false);

		if (IS_ENTITY_A_PED(entity))
			CLEAR_PED_TASKS_IMMEDIATELY(entity);
	}

	if (ComponentExists<EffectDispatcher>())
	{
		m_RunningEffects = GetComponent<EffectDispatcher>()->SharedState.ActiveEffects;

		GetComponent<EffectDispatcher>()->ClearActiveEffects("misc_fake_nochaos");
	}

	if (ComponentExists<MetaModifiers>())
		GetComponent<MetaModifiers>()->DisableChaos = true;

	CurrentEffect::OverrideEffectNameFromId("meta_nochaos");

	int waitTimer = g_Random.GetRandomInt(6000, 12000);

	int startTick = GET_GAME_TIMER();
	while (GET_GAME_TIMER() - startTick < waitTimer)
	{
		WAIT(0);
		auto percentage = 1.f -
		    (float)(GET_GAME_TIMER() - startTick) / 1000
		    / (ComponentExists<EffectDispatcher>() ? GetComponent<EffectDispatcher>()->SharedState.MetaEffectTimedDur : 90);
		CurrentEffect::OverrideEffectCompletionPercentage(percentage);
	}
	
	if (ComponentExists<MetaModifiers>())
	GetComponent<MetaModifiers>()->DisableChaos = false;
	
	if (ComponentExists<EffectDispatcher>())
	{
		for (size_t i = 0; i < m_RunningEffects.size(); i++)
		if (m_RunningEffects[i].IsTimed)
		GetComponent<EffectDispatcher>()->DispatchEffect(m_RunningEffects[i].Id);
		
		WAIT(100); // hack - wait for effect dispatcher to dispatch effects
		
		for (size_t i = 0; i < m_RunningEffects.size(); i++)
		{
			if (m_RunningEffects[i].IsTimed)
			{
				GetComponent<EffectDispatcher>()->SetRemainingTimeForEffect(m_RunningEffects[i].Id,
					m_RunningEffects[i].Timer);
				}
			}
		}
		
		for (const auto &entity : m_SpawnedEntities)
		{
			FREEZE_ENTITY_POSITION(entity, false);
			SET_ENTITY_ALPHA(entity, 255, false);
			CLEAR_PED_TASKS_IMMEDIATELY(entity);
		SET_ENTITY_COLLISION(entity, true, false);
	}

	CurrentEffect::OverrideEffectCompletionPercentage(-1.f);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr,
	{
		.Name = "Fake No Chaos",
		.Id = "misc_fake_nochaos",
		.IsTimed = false,
        .HideRealNameOnStart = true,
        .AlwaysShowName = true
	}
);