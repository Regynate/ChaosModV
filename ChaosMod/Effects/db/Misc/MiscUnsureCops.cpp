#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void OnTick()
{
	auto playerId          = PLAYER_ID();
	auto randomWantedLevel = GET_RANDOM_INT_IN_RANGE(0, 5);
	SET_PLAYER_WANTED_LEVEL(playerId, randomWantedLevel, false);
	SET_PLAYER_WANTED_LEVEL_NOW(playerId, false);

	WAIT(3000);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Unsure Cops", 
        .Id = "misc_unsure_cops", 
        .IsTimed = true
    }
);
