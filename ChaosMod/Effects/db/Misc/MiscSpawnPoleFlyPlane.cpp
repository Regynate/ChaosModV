#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

#include "Components/EffectDispatcher.h"
#include "Components/EntityTracking.h"

static void OnStart()
{
	if (CurrentEffect::GetDispatchContext() == "spawn")
	{
		const auto playerPed = PLAYER_PED_ID();
		const auto playerPos = GET_ENTITY_COORDS(playerPed, false);
		const auto playerVel = GET_ENTITY_VELOCITY(playerPed);

		auto coords          = Vector3(playerPos.x + playerVel.x * 0.2f, playerPos.y + playerVel.y * 0.2f, playerPos.z - 1.f);
		const auto pole      = CreatePoolProp(GET_HASH_KEY("prop_cctv_pole_01a"), coords.x, coords.y, coords.z, false);
		SET_DISABLE_BREAKING(pole, true);

		return;
	}

	if (ComponentExists<EntityTracking>())
	{
		GetComponent<EntityTracking>()->AddPoleSpawnFlyTracker();
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Spawn A Pole Next Time You Fly A Plane",
		.Id = "misc_pole_fly"
	}
);