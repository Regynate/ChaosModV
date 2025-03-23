#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static bool RequestControlEntity(Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(Entity entity)
{
	if (RequestControlEntity(entity))
	{
		if (!IS_ENTITY_A_MISSION_ENTITY(entity))
			SET_ENTITY_AS_MISSION_ENTITY(entity, true, true);
		DELETE_ENTITY(&entity);
	}
}

CHAOS_VAR std::unordered_map<Ped, Ped> affectedPeds;

static void GivePedsBlueBalls()
{
	for (auto const ped : GetAllPeds())
	{
		if (!DOES_ENTITY_EXIST(ped) || IS_PED_DEAD_OR_DYING(ped, false) || !IS_PED_HUMAN(ped))
			return;

		if (affectedPeds.contains(ped))
			return;

		SET_ENTITY_ALPHA(ped, 0, false);

		auto const blueBallModel = GET_HASH_KEY("prop_swiss_ball_01");
		LoadModel(blueBallModel);

		auto const pedCoords = GET_ENTITY_COORDS(ped, false);
		auto const blueBall  = CreatePoolProp(blueBallModel, pedCoords.x, pedCoords.y, pedCoords.z + 5, false);

		ATTACH_ENTITY_TO_ENTITY(blueBall, ped, 0, 0, 0, 0, 0, 0, 0, false, false, false, false, 0, true);
		SET_ENTITY_INVINCIBLE(blueBall, true);
		affectedPeds.emplace(ped, blueBall);
	}
	
}

static void CleanupBlueBalls()
{
	for (auto const &[ped, blueBall] : affectedPeds)
	{
		if (DOES_ENTITY_EXIST(blueBall))
		{
			DETACH_ENTITY(blueBall, true, true);
			DeleteEntity(blueBall);
		}
		if (DOES_ENTITY_EXIST(ped))
			SET_ENTITY_ALPHA(ped, 255, false);
	}
	affectedPeds.clear();
}

static void CheckPedsDistance()
{
	auto const player       = PLAYER_PED_ID();
	auto const playerCoords = GET_ENTITY_COORDS(player, false);

	for (auto it = affectedPeds.begin(); it != affectedPeds.end();)
	{
		auto const ped      = it->first;
		auto const blueBall = it->second;

		if (!DOES_ENTITY_EXIST(ped) || IS_PED_DEAD_OR_DYING(ped, false))
		{
			if (DOES_ENTITY_EXIST(blueBall))
			{
				DETACH_ENTITY(blueBall, true, true);
				DeleteEntity(blueBall);
			}
			it = affectedPeds.erase(it);
			continue;
		}

		auto const pedCoords = GET_ENTITY_COORDS(ped, false);
		auto const distance  = GET_DISTANCE_BETWEEN_COORDS(playerCoords.x, playerCoords.y, playerCoords.z, pedCoords.x,
		                                                   pedCoords.y, pedCoords.z, false);

		if (distance > 100.0f)
		{
			if (DOES_ENTITY_EXIST(blueBall))
			{
				DETACH_ENTITY(blueBall, true, true);
				DeleteEntity(blueBall);
			}
			SET_ENTITY_ALPHA(ped, 255, false);
			it = affectedPeds.erase(it);
		}
		else
		{
			++it;
		}
	}
}

static void OnStop()
{
	auto const player = PLAYER_PED_ID();
	if (IS_PED_DEAD_OR_DYING(player, false))
		SET_ENTITY_ALPHA(player, 255, false);

	CleanupBlueBalls();
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	if (IS_PED_DEAD_OR_DYING(player, false))
		SET_ENTITY_ALPHA(player, 255, false);

	GivePedsBlueBalls();
	CheckPedsDistance();
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
    {
        .Name = "BlueBall%",
        .Id = "peds_blue_balls", 
        .IsTimed = true
    }
);
