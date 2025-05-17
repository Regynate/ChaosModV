#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Memory/Physics.h"
#include "Memory/Vehicle.h"

static void OnTick()
{
	static const Hash blimpHash = "BLIMP"_hash;

	for (Vehicle veh : GetAllVehs())
	{
		Hash vehModel = GET_ENTITY_MODEL(veh);
		int vehClass  = GET_VEHICLE_CLASS(veh);

		// Exclude helis since the "braking" flag seems to be always set for those
		// Also manually exclude blimps since those don't seem to be categorized as either of those
		if (vehClass != 15 && vehModel != blimpHash && Memory::IsVehicleBraking(veh))
		{
			const auto speed = GET_ENTITY_SPEED_VECTOR(veh, true).y;
			SET_VEHICLE_FORWARD_SPEED(veh, speed + 50.f * GET_FRAME_TIME());
		}
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
	{
		.Name = "Brake Boosting",
		.Id = "veh_brakeboost",
		.IsTimed = true,
		.IncompatibleWith = { "veh_boostbrake" },
	}
);