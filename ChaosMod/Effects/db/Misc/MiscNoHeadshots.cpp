#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void ped(void (*processor)(std::int32_t))
{
	static constexpr std::int32_t MAX_ENTITIES = 100;
	std::int32_t peds[MAX_ENTITIES];
	auto const ped_count = worldGetAllPeds(peds, MAX_ENTITIES);

	for (std::int32_t const i : std::ranges::iota_view { 0, ped_count })
	{
		auto const ped_handle        = peds[i];

		auto const does_entity_exist = DOES_ENTITY_EXIST(ped_handle);
		if (!does_entity_exist)
			continue;

		processor(ped_handle);
	}
}

static void ExplodePlayerOnHeadshot(const std::int32_t pedHandle)
{
	auto constexpr HEAD = 31086;

	auto const player   = PLAYER_PED_ID();
	if (player == pedHandle)
		return;

	auto const sourceOfDeath = GET_PED_SOURCE_OF_DEATH(pedHandle);
	if (sourceOfDeath != player)
		return;

	int damagedBone {};
	GET_PED_LAST_DAMAGE_BONE(pedHandle, &damagedBone);

	if (damagedBone != HEAD)
		return;

	auto const playerCoords = GET_ENTITY_COORDS(player, false);

	ADD_EXPLOSION(playerCoords.x, playerCoords.y, playerCoords.z, 29, 100.f, true, false, 5.f, false);
	APPLY_DAMAGE_TO_PED(player, 1337, true, 0);

	CLEAR_PED_LAST_DAMAGE_BONE(pedHandle);
}

static void OnStart()
{
}

static void OnStop()
{
}

static void OnTick()
{
	ped(ExplodePlayerOnHeadshot);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "No Headshots", 
        .Id = "misc_no_headshots", 
        .IsTimed = true
    }
);