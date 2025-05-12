#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>

CHAOS_VAR auto constexpr grenadeCount = 5;
CHAOS_VAR std::unordered_set<Ped> processedPeds;


static void ShootGrenadesOnDeath()
{
	for (auto const ped : GetAllPeds())
	{
		if (!DOES_ENTITY_EXIST(ped) || !IS_PED_DEAD_OR_DYING(ped, false))
			continue;

		if (processedPeds.contains(ped))
			continue;

		processedPeds.insert(ped);
		auto const pedCoords   = GET_ENTITY_COORDS(ped, true);
		auto const grenadeHash = GET_HASH_KEY("weapon_grenade");
		
		for (auto i = 0; i < grenadeCount; i++)
		{
			auto const offsetX  = GET_RANDOM_FLOAT_IN_RANGE(-0.5f, 0.5f);
			auto const offsetY  = GET_RANDOM_FLOAT_IN_RANGE(-0.5f, 0.5f);
			auto const grenadeX = pedCoords.x + offsetX;
			auto const grenadeY = pedCoords.y + offsetY;
			auto const grenadeZ = pedCoords.z + 0.5f;

			auto const targetX  = grenadeX + GET_RANDOM_FLOAT_IN_RANGE(-2.0f, 2.0f);
			auto const targetY  = grenadeY + GET_RANDOM_FLOAT_IN_RANGE(-2.0f, 2.0f);
			auto const targetZ  = grenadeZ + GET_RANDOM_FLOAT_IN_RANGE(1.0f, 3.0f);

			SHOOT_SINGLE_BULLET_BETWEEN_COORDS(grenadeX, grenadeY, grenadeZ, targetX, targetY, targetZ, 100, true,
			                                   grenadeHash, ped, true, false, 1.0f);
		}
    }  
}

static void OnStart()
{
    auto const player = PLAYER_PED_ID();
    auto const grenadeHash = GET_HASH_KEY("weapon_grenade");
    GIVE_DELAYED_WEAPON_TO_PED(player, grenadeHash, 1, false);
    processedPeds.clear();
}

static void OnStop()
{
    processedPeds.clear();
}

static void OnTick()
{
	ShootGrenadesOnDeath();
}

REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "A Bad Year",
        .Id = "peds_a_bad_year",
        .IsTimed = true
    }
);
