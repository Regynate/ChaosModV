#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Util/Vehicle.h"

static void OnStart()
{
	for (auto const vehicle : GetAllVehs())
	{
		auto const player            = PLAYER_PED_ID();
		auto const coords            = GET_ENTITY_COORDS(player, false);

		auto const vehicleCoords     = GET_ENTITY_COORDS(vehicle, false);

		auto const distanceToVehicle = GET_DISTANCE_BETWEEN_COORDS(coords.x, coords.y, coords.z, vehicleCoords.x,
		                                                           vehicleCoords.y, vehicleCoords.z, false);

		if (distanceToVehicle > 125.f)
			continue;

		static std::array<Hash, 6> bikeModels = { GET_HASH_KEY("bati"),    GET_HASH_KEY("hexer"),
			                                              GET_HASH_KEY("daemon"),  GET_HASH_KEY("hakuchou"),
			                                              GET_HASH_KEY("zombiea"), GET_HASH_KEY("sovereign") };

		auto constexpr max                            = bikeModels.size();
		auto const randomBikeModel                    = GET_RANDOM_INT_IN_RANGE(0, max);
		auto const selectedModel                      = bikeModels[randomBikeModel];

		auto const bike = ReplaceVehicleWithModel(vehicle, selectedModel, true);
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Lost And The Damned", 
        .Id = "vehs_lost_and_damned", 
        .IsTimed = false
    }
);