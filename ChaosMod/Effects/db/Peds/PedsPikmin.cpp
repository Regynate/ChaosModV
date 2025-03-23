#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static bool RequestControlEntity(const std::uint32_t entity)
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

CHAOS_VAR Entity randomPed {};

static void OnStart()
{
	auto const player            = PLAYER_PED_ID();
	auto const behindCoordinates = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(player, -2.0f, -2.0f, 0.0f);

	randomPed                    = CREATE_RANDOM_PED(behindCoordinates.x, behindCoordinates.y, behindCoordinates.z);

	FREEZE_ENTITY_POSITION(randomPed, true);
	SET_ENTITY_ALPHA(randomPed, 0, false);
	ATTACH_ENTITY_TO_ENTITY(randomPed, player, 0, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, false, false, false, 0,
	                        true);
}

static void OnStop()
{
	DETACH_ENTITY(randomPed, true, true);
	DeleteEntity(randomPed);
}

static void OnTick()
{
	for (auto const ped : GetAllPeds())
	{
		auto const player            = PLAYER_PED_ID();
		auto const behindCoordinates = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(player, -2.0f, -2.0f, 0.0f);

		if (player == ped || !randomPed)
			return;

		TASK_GO_TO_ENTITY(ped, randomPed, 10000, 2.0f, 100.0f, 0, 0);
	}
	WAIT(2000);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Pikmin", 
        .Id = "peds_pikmin", 
        .IsTimed = true
    }
);
