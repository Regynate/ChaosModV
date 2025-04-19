#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static bool RequestControlEntity(Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(Entity entity)
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
	auto const playerId = PLAYER_ID();

	if (!IS_PED_SHOOTING(player))
		return;

	Entity entity{};
	GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(playerId, &entity);

	if (!DOES_ENTITY_EXIST(entity))
		return;

	DeleteEntity(entity);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Strong Guns", 
        .Id = "misc_strong_guns", 
        .IsTimed = true
    }
);
