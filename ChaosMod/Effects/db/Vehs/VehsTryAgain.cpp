#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

#include "Components/EntityTracking.h"

static void OnStart()
{
	if (!ComponentExists<EntityTracking>())
		return;

	auto const lastVehicleEntry = GetComponent<EntityTracking>()->GetLastPlayerVehicleEntryPoint();

	auto vehicle                = lastVehicleEntry.vehicle;
	auto const vehicleModel     = lastVehicleEntry.vehicleModel;
	auto const vehicleCoords    = lastVehicleEntry.vehicleCoords;
	auto const playerCoords     = lastVehicleEntry.playerCoords;
	auto const playerHeading    = lastVehicleEntry.playerHeading;
	auto const vehicleHeading   = lastVehicleEntry.vehicleHeading;
	auto const insideVehicle    = lastVehicleEntry.insideVehicle;

	auto const player           = PLAYER_PED_ID();
	auto const invalidCoords    = vehicleCoords.x == 0 && vehicleCoords.y == 0 && vehicleCoords.z == 0;

	if (invalidCoords)
		return;

	if (!DOES_ENTITY_EXIST(vehicle))
		vehicle = CreatePoolVehicle(vehicleModel, vehicleCoords.x, vehicleCoords.y, vehicleCoords.z, vehicleHeading);

	if (insideVehicle)
		SET_PED_INTO_VEHICLE(player, vehicle, -1);
	else
	{
		SET_ENTITY_COORDS(player, playerCoords.x, playerCoords.y, playerCoords.z, false, false, false, false);
		SET_ENTITY_HEADING(player, playerHeading);
	}

	SET_ENTITY_COORDS(vehicle, vehicleCoords.x, vehicleCoords.y, vehicleCoords.z, false, false, false, false);
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

