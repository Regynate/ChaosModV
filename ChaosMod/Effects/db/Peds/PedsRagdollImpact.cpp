#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"


CHAOS_VAR float launchForce = 75.0f;
CHAOS_VAR std::unordered_set<std::uint32_t> processedPeds;


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
	for (auto const ped : GetAllPeds())
	{
		if (!DOES_ENTITY_EXIST(ped) || !IS_PED_DEAD_OR_DYING(ped, false))
			continue;

		if (processedPeds.contains(ped))
			continue;

		processedPeds.insert(ped);

		auto const player            = PLAYER_PED_ID();
		auto const playerCoordinates = GET_ENTITY_COORDS(player, true);
		auto const pedCoordinates    = GET_ENTITY_COORDS(ped, true);

		auto const inVehicle         = IS_PED_IN_ANY_VEHICLE(ped, false);
		Vehicle vehicle {};
		if (inVehicle)
			vehicle = GET_VEHICLE_PED_IS_IN(ped, false);

		Vector3 direction { playerCoordinates.x - pedCoordinates.x, playerCoordinates.y - pedCoordinates.y,
			                (playerCoordinates.z + 2.0f) - pedCoordinates.z };

		auto const magnitude = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		if (magnitude == 0.0f)
			continue;

		direction.x /= magnitude;
		direction.y /= magnitude;
		direction.z /= magnitude;

		auto const force =
		    Vector3 { direction.x * launchForce, direction.y * launchForce, direction.z * launchForce + 2.0f };

		if (inVehicle)
		{
			APPLY_FORCE_TO_ENTITY(vehicle, 1, force.x, force.y, force.z, 0.0f, 0.0f, 0.0f, 0, false, true, true, false,
			                      true);
			continue;
		}

		APPLY_FORCE_TO_ENTITY(ped, 1, force.x, force.y, force.z, 0.0f, 0.0f, 0.0f, 0, false, true, true, false, true);
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Launch Peds Towards Player When Killed", 
        .Id = "peds_ragdoll_impact", 
        .IsTimed = true
    }
);
