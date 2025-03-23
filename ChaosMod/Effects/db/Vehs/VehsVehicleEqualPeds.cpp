#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

CHAOS_VAR std::array<Hash, 5> vehicleModels = { GET_HASH_KEY("adder"), GET_HASH_KEY("t20"),
	                                                 GET_HASH_KEY("banshee"), GET_HASH_KEY("zentorno"),
	                                                 GET_HASH_KEY("dominator") };

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
	auto constexpr maxEntities = 100;
	Ped peds[maxEntities], vehicles[maxEntities];

	for (auto const ped : GetAllPeds())
	{
		for (auto const vehicle : GetAllVehs())
		{
			auto maxCount = (ped > vehicle) ? ped : vehicle;

			for (int i = 0; i < maxCount; i++)
			{
				if (i < ped)
				{
					auto ped = peds[i];
					if (!DOES_ENTITY_EXIST(ped) || ped == PLAYER_PED_ID())
						continue;

					if (i < vehicle)
					{
						auto vehicle = vehicles[i];
						if (!DOES_ENTITY_EXIST(vehicle))
							continue;

						auto pedCoordinates     = GET_ENTITY_COORDS(ped, true);
						auto pedHeading         = GET_ENTITY_HEADING(ped);
						auto vehicleCoordinates = GET_ENTITY_COORDS(vehicle, false);
						auto randomModel        = vehicleModels[GET_RANDOM_INT_IN_RANGE(0, vehicleModels.size())];

						LoadModel(randomModel);
						DeleteEntity(ped);
						DeleteEntity(vehicle);

						CREATE_RANDOM_PED(vehicleCoordinates.x, vehicleCoordinates.y, vehicleCoordinates.z);
						CREATE_VEHICLE(randomModel, pedCoordinates.x, pedCoordinates.y, pedCoordinates.z, pedHeading,
						               true, true, false);
					}
				}
			}
		}	
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
