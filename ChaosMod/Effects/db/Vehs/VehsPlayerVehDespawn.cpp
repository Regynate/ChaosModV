#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

#include "Util/Vehicle.h"

static void OnStart()
{
	Ped playerPed = PLAYER_PED_ID();

	if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
	{
		Vehicle playerVeh = GET_VEHICLE_PED_IS_IN(playerPed, false);

		DeleteVehicle(playerVeh);
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Remove Current Vehicle",
		.Id = "playerveh_despawn"
	}
);