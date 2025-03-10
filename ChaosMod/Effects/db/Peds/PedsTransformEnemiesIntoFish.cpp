#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static bool RequestControlEntity(Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;

	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(std::int32_t entity)
{
	if (RequestControlEntity(entity))
	{
		if (!IS_ENTITY_A_MISSION_ENTITY(entity))
			SET_ENTITY_AS_MISSION_ENTITY(entity, true, true);
		DELETE_ENTITY(&entity);
	}
}

static void ped(void (*processor)(std::int32_t))
{
	static constexpr std::int32_t MAX_ENTITIES = 100;
	std::int32_t peds[MAX_ENTITIES];
	auto const pedCount = worldGetAllPeds(peds, MAX_ENTITIES);

	for (std::int32_t const i : std::ranges::iota_view { 0, pedCount })
	{
		auto const pedHandle = peds[i];

		if (!DOES_ENTITY_EXIST(pedHandle))
			continue;

		processor(pedHandle);
	}
}

static void ReplaceEnemiesWithFish(const std::int32_t pedHandle)
{
	auto constexpr fishModel      = GET_HASH_KEY("a_c_fish");

	auto const player = PLAYER_PED_ID();
	auto const inCombatWithPlayer = IS_PED_IN_COMBAT(pedHandle, player);
	if (!inCombatWithPlayer)
	{
		return;
	}
	auto const isDead      = IS_PED_DEAD_OR_DYING(pedHandle, false);
	if (isDead)
	{
		return;
	}

	auto const enemyCoords = GET_ENTITY_COORDS(pedHandle, false);
	auto const enemyHeading = GET_ENTITY_HEADING(pedHandle);
	
	DeleteEntity(pedHandle);

	CreatePoolPed(6, fishModel, enemyCoords.x, enemyCoords.y, enemyCoords.z, enemyHeading);
}
static void OnStart()
{
	ped(ReplaceEnemiesWithFish);
}

static void OnStop()
{
	
}

static void OnTick()
{

}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Enemies Are Fish",
        .Id = "peds_enemies_are_fish", 
        .IsTimed = false
    }
);
