#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR std::uint32_t currentTarget {};

static void OnStart()
{
	currentTarget = 0;
}

static void OnStop()
{
	currentTarget = 0;
}

static void RunTowardsTheNearestEnemy()
{
	for (auto const ped : GetAllPeds())
	{
		auto const player            = PLAYER_PED_ID();
		auto const playerCoordinates = GET_ENTITY_COORDS(player, false);

		Ped nearestEnemy             = 0;
		float nearestDistance        = 200.0f;

		if (ped == player || !DOES_ENTITY_EXIST(ped) || IS_PED_DEAD_OR_DYING(ped, true))
			continue;

		auto const inCombatWithPlayer = IS_PED_IN_COMBAT(ped, player);
		auto const relation           = GET_RELATIONSHIP_BETWEEN_PEDS(player, ped);
		auto const hatesPlayer        = relation == 4 || relation == 5;
		auto const pedType            = GET_PED_TYPE(ped);
		auto const isCop              = pedType == 6;
		if (hatesPlayer || inCombatWithPlayer || isCop)
		{
			auto const pedCoordinates = GET_ENTITY_COORDS(ped, false);
			auto const distance =
			    GET_DISTANCE_BETWEEN_COORDS(playerCoordinates.x, playerCoordinates.y, playerCoordinates.z,
			                                pedCoordinates.x, pedCoordinates.y, pedCoordinates.z, true);

			if (distance < nearestDistance)
			{
				nearestEnemy    = ped;
				nearestDistance = distance;
			}
		}

		if (DOES_ENTITY_EXIST(nearestEnemy) && nearestEnemy != currentTarget)
		{
			CLEAR_PED_TASKS(player);
			currentTarget = nearestEnemy;
			TASK_GO_TO_ENTITY(player, currentTarget, -1, 0.0f, 5.0f, 0, 0);
			WAIT(2000);
		}
	}
}


static void OnTick()
{
	RunTowardsTheNearestEnemy();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Run Towards Nearest Enemy", 
        .Id = "player_run_towards_nearest_enemy", 
        .IsTimed = true,
        .IsShortDuration = true
    }
);
