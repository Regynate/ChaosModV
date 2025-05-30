#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static bool RequestControlEntity(Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void OnTick()
{
	auto const player     = PLAYER_PED_ID();
	auto const isShooting = IS_PED_SHOOTING(player);
	if (!isShooting)
		return;

	auto const playerId = PLAYER_ID();

	Entity entity {};
	GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(playerId, &entity);

	auto const exists = DOES_ENTITY_EXIST(entity);
	if (!exists)
	{
		Vector3 impactCoords {};
		GET_PED_LAST_WEAPON_IMPACT_COORD(player, &impactCoords);

		if (impactCoords.x == 0 && impactCoords.y == 0 && impactCoords.z == 0)
			return;

		Entity toTeleport = player;

		if (IS_PED_IN_ANY_VEHICLE(player, false))
			toTeleport = GET_VEHICLE_PED_IS_IN(player, false);

		float heading = GET_ENTITY_HEADING(toTeleport);
		Vector3 vel   = GET_ENTITY_VELOCITY(toTeleport);
		float forward = GET_ENTITY_SPEED(toTeleport);

		SET_ENTITY_COORDS(toTeleport, impactCoords.x, impactCoords.y, impactCoords.z, true, true, true, false);
		SET_ENTITY_VELOCITY(toTeleport, vel.x, vel.y, vel.z);

		if (IS_ENTITY_A_VEHICLE(toTeleport))
			SET_VEHICLE_FORWARD_SPEED(toTeleport, forward);
		return;
	}

	if (!RequestControlEntity(entity))
		return;

	auto const coords = GET_ENTITY_COORDS(player, false);
	SET_ENTITY_COORDS(entity, coords.x, coords.y, coords.z, true, true, true, false);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Get Over Here!", 
        .Id = "misc_reverse_portal_guns", 
        .IsTimed = true
    }
);
