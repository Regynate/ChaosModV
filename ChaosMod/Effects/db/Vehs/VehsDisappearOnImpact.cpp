/*
    Effect by juhana
*/

#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

#include "Util/Vehicle.h"

static void OnTick()
{
	for (auto veh : GetAllVehs())
		if (HAS_ENTITY_COLLIDED_WITH_ANYTHING(veh))
			DeleteVehicle(veh);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick,
	{
		.Name = "Vehicles Disappear On Impact",
		.Id = "vehs_disappear_on_impact",
		.IsTimed = true
	}
);