#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>
// TO-DO: MAKE PEDS GO INTO DRIVERS SEAT OF NEWLY SPAWNED MOTORCYCLES


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

static void OnStart()
{
	for (auto const vehicle : GetAllVehs())
	{
		auto const player            = PLAYER_PED_ID();
		auto const coords            = GET_ENTITY_COORDS(player, false);

		auto const vehicleCoords     = GET_ENTITY_COORDS(vehicle, false);

		auto const distanceToVehicle = GET_DISTANCE_BETWEEN_COORDS(coords.x, coords.y, coords.z, vehicleCoords.x,
		                                                           vehicleCoords.y, vehicleCoords.z, false);

		if (distanceToVehicle > 150.f)
			continue;

		static std::array<std::int32_t, 6> bikeModels = { GET_HASH_KEY("bati"),    GET_HASH_KEY("hexer"),
			                                              GET_HASH_KEY("daemon"),  GET_HASH_KEY("hakuchou"),
			                                              GET_HASH_KEY("zombiea"), GET_HASH_KEY("sovereign") };

		auto constexpr max                            = bikeModels.size();
		auto const randomBikeModel                    = GET_RANDOM_INT_IN_RANGE(0, max);
		auto const selectedModel                      = bikeModels[randomBikeModel];

		LoadModel(selectedModel);

		auto const vehicleHeading = GET_ENTITY_HEADING(vehicle);

		auto const driver         = GET_PED_IN_VEHICLE_SEAT(vehicle, -1, 0);
		auto const driverCoords   = GET_ENTITY_COORDS(driver, false);

		SET_ENTITY_COORDS(driver, driverCoords.x, driverCoords.y, driverCoords.z + 10, false, false, false, false);

		DeleteEntity(vehicle);

		auto const bike =
		    CreatePoolVehicle(selectedModel, vehicleCoords.x, vehicleCoords.y, vehicleCoords.z, vehicleHeading);

		SET_PED_INTO_VEHICLE(driver, bike, -1);
	}
}

static void OnTick()
{
}

static void OnStop()
{
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Lost And The Damned", 
        .Id = "vehs_lost_and_damned", 
        .IsTimed = false
    }
);