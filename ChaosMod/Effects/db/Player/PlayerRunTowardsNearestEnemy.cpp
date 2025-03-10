#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void ped(void (*processor)(std::int32_t))
{
	static constexpr std::int32_t MAX_ENTITIES = 100;
	std::int32_t peds[MAX_ENTITIES];
	auto const pedCount = worldGetAllPeds(peds, MAX_ENTITIES);

	for (auto const i : std::ranges::iota_view { 0, pedCount })
	{
		auto const pedHandle = peds[i];

		if (!DOES_ENTITY_EXIST(pedHandle))
			continue;

		processor(pedHandle);
	}
}

static std::uint32_t currentTarget {};

static void OnStart()
{
	currentTarget = 0;
}

static void OnStop()
{
	currentTarget = 0;
}

static void RunTowardsTheNearestEnemy(const std::int32_t pedHandle)
{
	auto const player            = PLAYER_PED_ID();
	auto const playerCoordinates = GET_ENTITY_COORDS(player, false);

	if (pedHandle == player || !DOES_ENTITY_EXIST(pedHandle) || IS_PED_DEAD_OR_DYING(pedHandle, true))
		return;

	auto const isPedInCombat         = IS_PED_IN_COMBAT(pedHandle, player);
	if (!isPedInCombat)
	{
		return;
	}

	auto constexpr maxSearchDistance = 50.0f;
	std::uint32_t nearestEnemy       = 0;
	auto const pedCoordinates        = GET_ENTITY_COORDS(pedHandle, false);
	auto const distance = GET_DISTANCE_BETWEEN_COORDS(playerCoordinates.x, playerCoordinates.y, playerCoordinates.z,
	                                                  pedCoordinates.x, pedCoordinates.y, pedCoordinates.z, true);

	if (distance < maxSearchDistance)
		nearestEnemy = pedHandle;

	if (!DOES_ENTITY_EXIST(currentTarget) || IS_PED_DEAD_OR_DYING(currentTarget, true))
	{
		CLEAR_PED_TASKS(currentTarget);
		currentTarget = nearestEnemy;
	}

	if (!DOES_ENTITY_EXIST(currentTarget) || !IS_PED_HUMAN(currentTarget))
		return;

	auto const targetCoordinates = GET_ENTITY_COORDS(currentTarget, false);
	TASK_GO_TO_ENTITY(player, currentTarget, 10000, 0.0f, 100.0f, 0, 0);
	WAIT(10000);
}

static void OnTick()
{
	ped(RunTowardsTheNearestEnemy);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Run Towards Nearest Enemy", 
        .Id = "player_run_towards_nearest_enemy", 
        .IsTimed = true
    }
);
