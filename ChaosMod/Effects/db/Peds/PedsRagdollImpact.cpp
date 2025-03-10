#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static constexpr float launchForce = 75.0f;
static std::unordered_set<std::uint32_t> processedPeds;

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

static void LaunchDeadNPCsAtPlayer(const std::int32_t ped)
{
	if (!DOES_ENTITY_EXIST(ped) || !IS_PED_DEAD_OR_DYING(ped, false))
		return;

	if (processedPeds.contains(ped))
		return;

	processedPeds.insert(ped);

	auto const player            = PLAYER_PED_ID();
	auto const playerCoordinates = GET_ENTITY_COORDS(player, true);
	auto const pedCoordinates    = GET_ENTITY_COORDS(ped, true);

	Vector3 direction { playerCoordinates.x - pedCoordinates.x, playerCoordinates.y - pedCoordinates.y,
		                (playerCoordinates.z + 2.0f) - pedCoordinates.z };

	auto const magnitude = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	if (magnitude == 0.0f)
		return;

	direction.x /= magnitude;
	direction.y /= magnitude;
	direction.z /= magnitude;

	auto const force =
	    Vector3 { direction.x * launchForce, direction.y * launchForce, direction.z * launchForce + 2.0f };

	APPLY_FORCE_TO_ENTITY(ped, 1, force.x, force.y, force.z, 0.0f, 0.0f, 0.0f, 0, false, true, true, false, true);
}

static void OnStart()
{
	processedPeds.clear();
}

static void OnStop()
{
	processedPeds.clear();
}

static void OnTick()
{
	ped(LaunchDeadNPCsAtPlayer);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Ragdoll Impact", 
        .Id = "peds_ragdoll_impact", 
        .IsTimed = true
    }
);
