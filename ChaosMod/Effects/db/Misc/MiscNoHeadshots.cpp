#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void ExplodePlayerOnHeadshot()
{
	for (auto const ped : GetAllPeds())
	{
		auto constexpr HEAD = 31086;

		auto const player   = PLAYER_PED_ID();
		if (player == ped)
			continue;

		auto const sourceOfDeath = GET_PED_SOURCE_OF_DEATH(ped);
		if (sourceOfDeath != player)
			continue;

		int damagedBone{};
		GET_PED_LAST_DAMAGE_BONE(ped, &damagedBone);

		if (damagedBone != HEAD)
			continue;

		auto const playerCoords = GET_ENTITY_COORDS(player, false);

		ADD_EXPLOSION(playerCoords.x, playerCoords.y, playerCoords.z, 29, 100.f, true, false, 5.f, false);
		APPLY_DAMAGE_TO_PED(player, 1337, true, 0);

		CLEAR_PED_LAST_DAMAGE_BONE(ped);
	}
}

static void OnTick()
{
	ExplodePlayerOnHeadshot();
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "No Headshots", 
        .Id = "misc_no_headshots", 
        .IsTimed = true
    }
);