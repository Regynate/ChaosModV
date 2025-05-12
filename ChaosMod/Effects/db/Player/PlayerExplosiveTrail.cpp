#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	auto const playerPed = PLAYER_PED_ID();
	SET_ENTITY_PROOFS(playerPed, false, false, true, false, false, false, false, false);
}

static void OnStop()
{
	auto const playerPed = PLAYER_PED_ID();
	SET_ENTITY_PROOFS(playerPed, false, false, false, false, false, false, false, false);
}

static void OnTick()
{
	auto const playerPed       = PLAYER_PED_ID();

	auto const isPedRunning = IS_PED_SPRINTING(playerPed);
	auto const isInVehicle = IS_PED_IN_ANY_VEHICLE(playerPed, false);
	if (!isPedRunning && !isInVehicle)
		return;

	auto const coords = GET_ENTITY_COORDS(playerPed, false);

	ADD_EXPLOSION(coords.x, coords.y, coords.z - 0.5, 2, 0.25f, false, false, 0.0f, true);

	WAIT(500);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Explosive Trail",
        .Id = "player_explosive_trail", 
        .IsTimed = true
    }
);