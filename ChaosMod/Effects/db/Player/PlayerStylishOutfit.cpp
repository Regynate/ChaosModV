#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void GiveFranklinStylishSuit()
{
	auto const player = PLAYER_PED_ID();
	SET_PED_COMPONENT_VARIATION(player, 3, 23, 1, 0);
	SET_PED_COMPONENT_VARIATION(player, 4, 15, 1, 0);
	SET_PED_COMPONENT_VARIATION(player, 6, 8, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, 8, 15, 1, 0);
	SET_PED_COMPONENT_VARIATION(player, 11, 4, 1, 0);
}

static void GiveTrevorStylishSuit()
{
	auto const player = PLAYER_PED_ID();
	SET_PED_COMPONENT_VARIATION(player, 3, 27, 1, 0);
	SET_PED_COMPONENT_VARIATION(player, 4, 20, 1, 0);
	SET_PED_COMPONENT_VARIATION(player, 6, 9, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, 8, 14, 0, 0);
}

static void GiveMichaelStylishSuit()
{
	auto const player = PLAYER_PED_ID();
	SET_PED_COMPONENT_VARIATION(player, 3, 0, 9, 0);
	SET_PED_COMPONENT_VARIATION(player, 4, 0, 9, 0);
}

static void GivePlayerStylishSuit()
{
	auto const player        = PLAYER_PED_ID();
	auto const playerModel   = GET_ENTITY_MODEL(player);

	auto const michaelModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	switch (playerModel)
	{
	case michaelModel:
		GiveMichaelStylishSuit();
		break;
	case franklinModel:
		GiveFranklinStylishSuit();
		break;
	case trevorModel:
		GiveTrevorStylishSuit();
		break;
	}
}

static void OnStart()
{
	GivePlayerStylishSuit();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Stylish Outfit", 
        .Id = "player_stylish_outfit", 
        .IsTimed = false
    }
);
