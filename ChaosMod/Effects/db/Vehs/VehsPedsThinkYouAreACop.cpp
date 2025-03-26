#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void OnTick()
{
	for (auto const vehicle : GetAllVehs())
	{
		auto const player           = PLAYER_PED_ID();
		auto const coords           = GET_ENTITY_COORDS(player, false);
		auto const vehicleCoords    = GET_ENTITY_COORDS(vehicle, false);
		auto const distanceToPlayer = GET_DISTANCE_BETWEEN_COORDS(coords.x, coords.y, coords.z, vehicleCoords.x,
		                                                          vehicleCoords.y, vehicleCoords.z, false);
		auto const driver           = GET_PED_IN_VEHICLE_SEAT(vehicle, -1, false);
		auto const vehicleHeading   = GET_ENTITY_HEADING(vehicle);
		auto const playerHeading    = GET_ENTITY_HEADING(player);
		auto const speed            = GET_ENTITY_SPEED(vehicle);

		auto const driverExists     = DOES_ENTITY_EXIST(driver);
		if (!driverExists)
			continue;

		auto const playersVehicle = GET_VEHICLE_PED_IS_IN(player, false);
		if (vehicle == playersVehicle)
			continue;

		if (distanceToPlayer < 70.f)
		{
			auto const model = GET_ENTITY_MODEL(vehicle);

			TASK_VEHICLE_MISSION(driver, vehicle, myVehicle, 22, 10.f, 1074528293, 10.f, -1.f, true);
			SET_VEHICLE_FORWARD_SPEED(vehicle, speed * 0.99);
			WAIT(10);
		}
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick,
	{
		.Name = "Peds Think You Are A Cop",
		.Id = "ped_peds_think_you_are_a_cop",
		.IsTimed = true,
	}
);
