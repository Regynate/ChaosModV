#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>
// TO-DO: MAKE PEDS GO INTO DRIVERS SEAT OF NEWLY SPAWNED MOTORCYCLES

static void vehicle(void (*processor)(std::int32_t))
{
	static constexpr std::int32_t MAX_ENTITIES = 100;

	std::int32_t vehicles[MAX_ENTITIES];
	auto const vehicles_count = worldGetAllVehicles(vehicles, MAX_ENTITIES);

	for (std::int32_t const i : std::ranges::iota_view { 0, vehicles_count })
	{
		auto const ped_handle        = vehicles[i];

		auto const does_entity_exist = ENTITY::DOES_ENTITY_EXIST(ped_handle);
		if (!does_entity_exist)
			continue;

		processor(ped_handle);
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

static void ReplaceVehicleWithBike(const std::int32_t vehicleHandle)
{

	static std::array<std::int32_t, 6> bikeModels = { GET_HASH_KEY("bati"), GET_HASH_KEY("hexer"),
		                                             GET_HASH_KEY("daemon"),  GET_HASH_KEY("hakuchou"),
		                                             GET_HASH_KEY("zombiea"),    GET_HASH_KEY("sovereign")
	};

	auto constexpr max                           = bikeModels.size() - 1;
	auto const randomBikeModel                    = GET_RANDOM_INT_IN_RANGE(0, max);
	auto const selectedModel                     = bikeModels[randomBikeModel];

	LoadModel(selectedModel);

	DeleteEntity(vehicleHandle);

	auto const vehicleCoords  = GET_ENTITY_COORDS(vehicleHandle, false);
	auto const vehicleHeading = GET_ENTITY_HEADING(vehicleHandle);

	auto const driver         = GET_PED_IN_VEHICLE_SEAT(vehicleHandle, -1, 0);
	auto const driverCoords   = GET_ENTITY_COORDS(driver, false);

	SET_ENTITY_COORDS(driver, driverCoords.x, driverCoords.y, driverCoords.z + 10, false, false, false, false);

	auto const bike = CreatePoolVehicle(selectedModel, vehicleCoords.x, vehicleCoords.y, vehicleCoords.z, vehicleHeading);
	
	SET_PED_INTO_VEHICLE(driver, bike, -1);
}

static void OnStart()
{
	vehicle(ReplaceVehicleWithBike);
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