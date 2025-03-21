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
	DELETE_ENTITY(&entity);
	
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
	auto constexpr copPedType   = 6;

	static auto const fishModel      = GET_HASH_KEY("a_c_fish");

	auto const player = PLAYER_PED_ID();

	auto const isDead      = IS_PED_DEAD_OR_DYING(pedHandle, false);
	if (isDead)
		return;

	auto const inCombatWithPlayer = IS_PED_IN_COMBAT(pedHandle, player);
	auto const relation           = GET_RELATIONSHIP_BETWEEN_PEDS(player, pedHandle);
	auto const hatesPlayer    = relation == 4 || relation == 5;
	auto const pedType            = GET_PED_TYPE(pedHandle);
	auto const isCop              = pedType == copPedType;
	if (hatesPlayer || inCombatWithPlayer || isCop)
	{
		auto const enemyCoords  = GET_ENTITY_COORDS(pedHandle, false);
		auto const enemyHeading = GET_ENTITY_HEADING(pedHandle);

		APPLY_DAMAGE_TO_PED(pedHandle, 69420, false, 0);
		SET_ENTITY_COORDS(pedHandle, 0, 0, 0, false, false, false, true);

		CreatePoolPed(28, fishModel, enemyCoords.x, enemyCoords.y, enemyCoords.z, enemyHeading);
	}	
}

static void OnStart()
{
	ped(ReplaceEnemiesWithFish);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Enemies Are Fish",
        .Id = "peds_enemies_are_fish", 
        .IsTimed = false
    }
);
