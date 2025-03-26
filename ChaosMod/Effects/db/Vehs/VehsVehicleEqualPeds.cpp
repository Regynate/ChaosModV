#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>
#include <ranges>

CHAOS_VAR std::array<Hash, 5> vehicleModels = { GET_HASH_KEY("adder"), GET_HASH_KEY("t20"), GET_HASH_KEY("banshee"),
	                                            GET_HASH_KEY("zentorno"), GET_HASH_KEY("dominator") };

static bool RequestControlEntity(Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(Entity entity)
{
	if (!RequestControlEntity(entity))
		return;
	if (!IS_ENTITY_A_MISSION_ENTITY(entity))
		SET_ENTITY_AS_MISSION_ENTITY(entity, true, true);
	DELETE_ENTITY(&entity);
}

static void ReplacePedsAndVehicles()
{
	#undef min

	auto const player           = PLAYER_PED_ID();
	auto const playerCoords     = GET_ENTITY_COORDS(player, false);
	auto constexpr maxDistance = 100.0f;

	std::vector<Ped> validPeds;
	std::vector<Vehicle> validVehicles;

	for (auto const ped : GetAllPeds())
	{
		if (ped == player || !DOES_ENTITY_EXIST(ped))
			continue;

		auto const pedCoords = GET_ENTITY_COORDS(ped, false);
		float distanceToPed =
		    VDIST2(playerCoords.x, playerCoords.y, playerCoords.z, pedCoords.x, pedCoords.y, pedCoords.z);

		if (distanceToPed <= maxDistance * maxDistance)
			validPeds.emplace_back(ped);
	}

	for (auto const vehicle : GetAllVehs())
	{
		if (!DOES_ENTITY_EXIST(vehicle))
			continue;

		auto const vehicleCoords = GET_ENTITY_COORDS(vehicle, false);
		float distanceToVehicle =
		    VDIST2(playerCoords.x, playerCoords.y, playerCoords.z, vehicleCoords.x, vehicleCoords.y, vehicleCoords.z);

		if (distanceToVehicle <= maxDistance * maxDistance)
			validVehicles.emplace_back(vehicle);
	}

	auto const minCount = std::min(validPeds.size(), validVehicles.size());

	for (auto const i : std::ranges::iota_view { std::size_t(0), minCount })
	{
		auto const ped           = validPeds[i];
		auto const vehicle       = validVehicles[i];

		auto const pedCoords     = GET_ENTITY_COORDS(ped, true);
		auto const pedHeading    = GET_ENTITY_HEADING(ped);
		auto const vehicleCoords = GET_ENTITY_COORDS(vehicle, false);
		auto const randomModel   = vehicleModels[GET_RANDOM_INT_IN_RANGE(0, vehicleModels.size())];

		DeleteEntity(ped);
		DeleteEntity(vehicle);

		CreateRandomPoolPed(vehicleCoords.x, vehicleCoords.y, vehicleCoords.z, pedHeading);
		CreatePoolVehicle(randomModel, pedCoords.x, pedCoords.y, pedCoords.z, pedHeading);
	}
}

static void OnStart()
{
	ReplacePedsAndVehicles();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Vehicles = Peds", 
        .Id = "vehs_vehicles_equal_peds", 
        .IsTimed = false
    }
);
