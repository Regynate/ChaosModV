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
	DELETE_ENTITY(&entity);
	
}

static void OnStart()
{
	for (auto const ped : GetAllPeds())
	{
		auto constexpr copPedType   = 6;

		static auto const fishModel = GET_HASH_KEY("a_c_fish");

		auto const player           = PLAYER_PED_ID();

		auto const isDead           = IS_PED_DEAD_OR_DYING(ped, false);
		if (isDead)
			continue;

		auto const inCombatWithPlayer = IS_PED_IN_COMBAT(ped, player);
		auto const relation           = GET_RELATIONSHIP_BETWEEN_PEDS(player, ped);
		auto const hatesPlayer        = relation == 4 || relation == 5;
		auto const pedType            = GET_PED_TYPE(ped);
		auto const isCop              = pedType == copPedType;
		if (hatesPlayer || inCombatWithPlayer || isCop)
		{
			auto const enemyCoords  = GET_ENTITY_COORDS(ped, false);
			auto const enemyHeading = GET_ENTITY_HEADING(ped);

			APPLY_DAMAGE_TO_PED(ped, 69420, false, 0);
			SET_ENTITY_COORDS(ped, 0, 0, 0, false, false, false, true);

			CreatePoolPed(28, fishModel, enemyCoords.x, enemyCoords.y, enemyCoords.z, enemyHeading);
		}
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Enemies Are Fish",
        .Id = "peds_enemies_are_fish", 
        .IsTimed = false
    }
);
