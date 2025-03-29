#include <stdafx.h>
#include "Util/Vehicle.h"
#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{	
	auto const [vehicle, vehicleHeading, playerHeading, vehicleCoords, playerCoords] = GetLastVehicleEntryCoords();
	
	auto const player = PLAYER_PED_ID();
	auto const invalidCoords = vehicleCoords.x == 0 && vehicleCoords.y == 0 && vehicleCoords.z == 0;

	if (invalidCoords)
		return;

	SET_ENTITY_COORDS(player, playerCoords.x, playerCoords.y, playerCoords.z, false, false, false, false);
	SET_ENTITY_COORDS(vehicle, vehicleCoords.x, vehicleCoords.y, vehicleCoords.z, false, false, false, false);

	SET_ENTITY_HEADING(player, playerHeading);
	SET_ENTITY_HEADING(vehicle, vehicleHeading);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Try Again",
		.Id = "veh_try_again",
        .EffectGroupType = EffectGroupType::Teleport
	}
);

