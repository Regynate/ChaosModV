#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	for (Vehicle veh : GetAllVehs())
		SET_VEHICLE_DOORS_LOCKED(veh, 2);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Lock All Vehicles",
		.Id = "vehs_lockdoors"
	}
);