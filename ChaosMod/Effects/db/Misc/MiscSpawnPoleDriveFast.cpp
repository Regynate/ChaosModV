#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

#include "Components/EffectDispatcher.h"
#include "Components/EntityTracking.h"

CHAOS_VAR const std::string configKey = "PLD";

static bool Tracker(std::any& data)
{
	bool &init           = *std::any_cast<bool>(&data);
	const auto playerPed = PLAYER_PED_ID();

	if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
	{
		const auto vehicle = GET_VEHICLE_PED_IS_IN(playerPed, false);
		if (GET_ENTITY_SPEED(vehicle) > 35.f)
		{
			if (!init)
			{
				if (ComponentExists<EffectDispatcher>())
				{
					GetComponent<EffectDispatcher>()->DispatchEffect(EffectIdentifier("misc_pole_drivefast"),
					                                                 EffectDispatcher::DispatchEffectFlag_None, "",
					                                                 "spawn");
					return false;
				}
			}
		}
		else
			init = false;
	}
	else
		init = false;

	return true;
}

static void OnStart()
{
	if (CurrentEffect::GetDispatchContext() == "spawn")
	{
		const auto playerPed = PLAYER_PED_ID();
		const auto playerPos = GET_ENTITY_COORDS(playerPed, false);
		const auto playerVel = GET_ENTITY_VELOCITY(playerPed);

		auto coords = Vector3(playerPos.x + playerVel.x * 0.2f, playerPos.y + playerVel.y * 0.2f, playerPos.z - 1.f);
		const auto pole = CreatePoolProp(GET_HASH_KEY("prop_cctv_pole_01a"), coords.x, coords.y, coords.z, false);
		SET_DISABLE_BREAKING(pole, true);
	}
	else
	{
		if (ComponentExists<Tracking>())
			GetComponent<Tracking>()->AddTracker(Tracker, true, configKey);
	}
}

static void OnInit()
{
	if (ComponentExists<Tracking>())
		GetComponent<Tracking>()->AddTrackerByConfigValue(configKey, Tracker, true);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, OnInit, 
	{
		.Name = "Spawn A Pole Next Time You Drive Fast",
		.Id = "misc_pole_drivefast"
	}
);