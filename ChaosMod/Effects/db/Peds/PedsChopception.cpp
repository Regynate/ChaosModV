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

struct ChopPed
{
	std::int32_t originalPed;
	std::int32_t attachedChop;
	Vector3 originalCoords;
};

static std::vector<ChopPed> affectedPeds;

static void AttachChopToPed(const std::int32_t ped)
{
	auto const chopModel = GET_HASH_KEY("A_C_Chop");

	if (!DOES_ENTITY_EXIST(ped) || IS_PED_DEAD_OR_DYING(ped, true) || IS_PED_A_PLAYER(ped) || !IS_PED_HUMAN(ped))
		return;

	if (std::any_of(affectedPeds.begin(), affectedPeds.end(),
	                [&](const ChopPed &entry) { return entry.originalPed == ped; }))
		return;

	LoadModel(chopModel);

	auto const pedCoords    = GET_ENTITY_COORDS(ped, true);
	SET_ENTITY_COORDS(ped, pedCoords.x, pedCoords.y, pedCoords.z, false, false, false, true);

	auto const heading = GET_ENTITY_HEADING(ped);

	auto const chop = CreatePoolPed(28, chopModel, pedCoords.x, pedCoords.y, pedCoords.z, heading);
	SET_ENTITY_AS_MISSION_ENTITY(chop, true, true);

	SET_ENTITY_VISIBLE(ped, false, false);
	ATTACH_ENTITY_TO_ENTITY(ped, chop, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, false, false, true, 0, true);

	affectedPeds.emplace_back(ChopPed { ped, chop, pedCoords });
}

static void RestorePeds()
{
	for (auto const &entry : affectedPeds)
	{
		if (DOES_ENTITY_EXIST(entry.originalPed) && DOES_ENTITY_EXIST(entry.attachedChop))
		{
			auto const chopCoords = GET_ENTITY_COORDS(entry.attachedChop, true);
			DETACH_ENTITY(entry.originalPed, true, true);
			SET_ENTITY_VISIBLE(entry.originalPed, true, false);
			SET_ENTITY_COORDS(entry.originalPed, chopCoords.x, chopCoords.y, chopCoords.z + 1, false, false, false, true);
			DeleteEntity(entry.attachedChop);
		}
	}
	affectedPeds.clear();
}

static void CheckChopDeaths()
{
	for (auto it = affectedPeds.begin(); it != affectedPeds.end();)
	{
		if (DOES_ENTITY_EXIST(it->attachedChop) && IS_PED_DEAD_OR_DYING(it->attachedChop, true))
		{
			if (DOES_ENTITY_EXIST(it->originalPed))
				SET_ENTITY_HEALTH(it->originalPed, 0, 0);
			DeleteEntity(it->attachedChop);
			it = affectedPeds.erase(it);
		}
		else
		{
			++it;
		}
	}
}

static void CheckPedsDistance()
{
	auto const player       = PLAYER_PED_ID();
	auto const playerCoords = GET_ENTITY_COORDS(player, false);

	for (auto it = affectedPeds.begin(); it != affectedPeds.end();)
	{
		auto const ped  = it->originalPed;
		auto const chop = it->attachedChop;

		if (!DOES_ENTITY_EXIST(ped) || !DOES_ENTITY_EXIST(chop))
		{
			it = affectedPeds.erase(it);
			continue;
		}

		auto const pedCoords = GET_ENTITY_COORDS(ped, false);
		auto const distance  = GET_DISTANCE_BETWEEN_COORDS(playerCoords.x, playerCoords.y, playerCoords.z, pedCoords.x,
		                                                   pedCoords.y, pedCoords.z, false);

		if (distance > 100.0f)
		{
			DETACH_ENTITY(ped, true, true);
			SET_ENTITY_VISIBLE(ped, true, false);
			SET_ENTITY_COORDS(ped, pedCoords.x, pedCoords.y, pedCoords.z, false, false, false, true);
			DeleteEntity(chop);
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
	RestorePeds();
}

static void OnTick()
{
	ped(AttachChopToPed);
	CheckChopDeaths();
	CheckPedsDistance();
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
    {
        .Name = "Chopception",
        .Id = "peds_chopception", 
        .IsTimed = true
    }
);
