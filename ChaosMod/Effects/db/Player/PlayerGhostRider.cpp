#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Memory/Physics.h"

static void OnTick()
{
	auto constexpr IgnoreBeingOnFire = 430;
	auto const player                = PLAYER_PED_ID();

	SET_PED_CONFIG_FLAG(player, IgnoreBeingOnFire, true);

	auto const vehicle = GET_VEHICLE_PED_IS_IN(player, false);
	SET_ENTITY_PROOFS(vehicle, false, true, false, false, false, false, false, false);

	SET_ENTITY_HEALTH(player, 200, 0);

	if (!IS_ENTITY_ON_FIRE(player))
		START_ENTITY_FIRE(player);

}

static void OnStop()
{
	auto const player = PLAYER_PED_ID();

	auto const vehicle = GET_VEHICLE_PED_IS_IN(player, false);
	SET_ENTITY_PROOFS(vehicle, false, false, false, false, false, false, false, false);
	SET_ENTITY_HEALTH(player, 200, 0);

	STOP_ENTITY_FIRE(player);
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
	{
		.Name = "Ghost Rider",
		.Id = "player_ghost_rider",
		.IsTimed = true,
	}
);