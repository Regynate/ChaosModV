#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

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

static std::unordered_map<std::int32_t, std::int32_t> affectedPeds;

static void GivePedsBlueBalls(const std::int32_t pedHandle)
{
	if (!DOES_ENTITY_EXIST(pedHandle) || IS_PED_DEAD_OR_DYING(pedHandle, false) || !IS_PED_HUMAN(pedHandle))
		return;

	if (affectedPeds.contains(pedHandle))
		return;

	SET_ENTITY_ALPHA(pedHandle, 0, false);

	auto const blueBallModel = GET_HASH_KEY("prop_swiss_ball_01");
	LoadModel(blueBallModel);

	auto const pedCoords = GET_ENTITY_COORDS(pedHandle, false);
	auto const blueBall  = CreatePoolProp(blueBallModel, pedCoords.x, pedCoords.y, pedCoords.z + 5, false);

	ATTACH_ENTITY_TO_ENTITY(blueBall, pedHandle, 0, 0, 0, 0, 0, 0, 0, false, false, false, false, 0, true);
	SET_ENTITY_INVINCIBLE(blueBall, true);
	affectedPeds.emplace(pedHandle, blueBall);
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

static void OnStart()
{
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

	ped(GivePedsBlueBalls);
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
