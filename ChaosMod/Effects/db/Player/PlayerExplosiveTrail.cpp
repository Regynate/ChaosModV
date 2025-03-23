#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void OnStart()
{
	auto const player = PLAYER_PED_ID();
	SET_ENTITY_PROOFS(player, false, false, true, false, false, false, false, false);
}

static void OnStop()
{
	auto const player = PLAYER_PED_ID();
	SET_ENTITY_PROOFS(player, false, false, false, false, false, false, false, false);
}

static void OnTick()
{
	auto const player       = PLAYER_PED_ID();

	auto const isPedRunning = IS_PED_SPRINTING(player);
	if (!isPedRunning)
		return;

	auto const coords = GET_ENTITY_COORDS(player, false);

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