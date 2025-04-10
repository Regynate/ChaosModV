#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void ExplodePlayerOnHeadshot()
{
	auto constexpr HEAD = 31086;

	auto const player   = PLAYER_PED_ID();

	for (auto const ped : GetAllPeds())
	{
		if (player == ped)
			continue;

		auto const sourceOfDeath = GET_PED_SOURCE_OF_DEATH(ped);
		if (sourceOfDeath != player)
			continue;

		int damagedBone {};
		GET_PED_LAST_DAMAGE_BONE(ped, &damagedBone);

		if (damagedBone != HEAD)
			continue;

		auto const boneCoords = GET_PED_BONE_COORDS(player, HEAD, 0.0, 0.0, 0.0);
		ADD_EXPLOSION(boneCoords.x, boneCoords.y, boneCoords.z, 5, 9999.f, true, false, 1.f, false);
		SET_ENTITY_HEALTH(player, 0, 0);

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