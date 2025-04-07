#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnTick()
{
	auto const player = PLAYER_PED_ID();

	if (!IS_PED_WALKING(player) || IS_PED_RAGDOLL(player))
		return;

	APPLY_FORCE_TO_ENTITY(player, 1, 0, 120, 0, 0, 0, 0, 0, true, true, false, false, false);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Slide Walk",
        .Id = "player_walking_speed_100x", 
        .IsTimed = true
    }
);
