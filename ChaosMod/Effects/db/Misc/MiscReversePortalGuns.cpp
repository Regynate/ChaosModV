#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static bool RequestControlEntity(std::int32_t entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(std::int32_t entity)
{
	if (!RequestControlEntity(entity))
		return;
	if (!IS_ENTITY_A_MISSION_ENTITY(entity))
		SET_ENTITY_AS_MISSION_ENTITY(entity, true, true);
	DELETE_ENTITY(&entity);
}

static void OnTick()
{
	auto const player   = PLAYER_PED_ID();
	auto const isShooting = IS_PED_SHOOTING(player);
	if (!isShooting)
		return;

	auto const playerId = PLAYER_ID();

	Entity entity{};
	GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(playerId, &entity);

	auto const exists = DOES_ENTITY_EXIST(entity);
	if (!exists)
	{
		Vector3 impactCoords{};
		GET_PED_LAST_WEAPON_IMPACT_COORD(player, &impactCoords);

		if (impactCoords.x == 0, impactCoords.y == 0, impactCoords.z == 0)
			return;

		SET_ENTITY_COORDS(player, impactCoords.x, impactCoords.y, impactCoords.z, false, false, false, false);
		return;	
	}

	if (!RequestControlEntity(entity))
		return;

	auto const coords = GET_ENTITY_COORDS(player, false);
	SET_ENTITY_COORDS(entity, coords.x, coords.y, coords.z, false, false, false, false);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Reverse Portal Guns", 
        .Id = "misc_reverse_portal_guns", 
        .IsTimed = true
    }
);
