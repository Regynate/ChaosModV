#include <stdafx.h>
#include <Components/EffectDispatcher.h>

static void OnStart()
{
	Ped playerPed = PLAYER_PED_ID();

	if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
	{
		Vehicle playerVeh = GET_VEHICLE_PED_IS_IN(playerPed, false);
		SET_VEHICLE_ENGINE_HEALTH(playerVeh, -1.f);
		SET_VEHICLE_PETROL_TANK_HEALTH(playerVeh, -1.f);
		SET_VEHICLE_BODY_HEALTH(playerVeh, -1.f);
	}
	else
	{
		GetComponent<EffectDispatcher>()->OverrideEffectNameId("player_ignite", "player_ignite");

		START_ENTITY_FIRE(playerPed);

		int startTick = GET_GAME_TIMER();

		while (GET_GAME_TIMER() - startTick < 20000)
		{
			LOG(GET_ENTITY_HEALTH(playerPed));

			if (GET_ENTITY_HEALTH(playerPed) < 110)
			{
				STOP_ENTITY_FIRE(playerPed);
				break;
			}

			WAIT(0);
		}
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, EffectInfo
	{
		.Name = "Ignite Player",
		.Id = "player_ignite",
		.IncompatibleWith = { "player_invincible" }
	}
);