#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

#include "Components/EntityTracking.h"

struct VehicleEntryPoint
{
	Vehicle vehicle;
	Hash vehicleModel;
	float vehicleHeading;
	float playerHeading;
	Vector3 vehicleCoords;
	Vector3 playerCoords;
	bool insideVehicle;
	bool updateFlag = false;
} entryPoint;

void UpdateVehicleEntryPoint()
{
	auto const playerPed = PLAYER_PED_ID();

	if (!DOES_ENTITY_EXIST(playerPed))
		return;

	auto const inVehicle       = IS_PED_IN_ANY_VEHICLE(playerPed, false);
	auto const enteringVehicle = IS_PED_IN_ANY_VEHICLE(playerPed, true);

	if (!enteringVehicle)
	{
		entryPoint.updateFlag = true;
		return;
	}

	auto const vehicle = GET_VEHICLE_PED_IS_USING(playerPed);

	if (entryPoint.vehicle != vehicle || entryPoint.updateFlag)
	{
		entryPoint.updateFlag     = false;
		entryPoint.vehicle        = vehicle;
		entryPoint.vehicleModel   = GET_ENTITY_MODEL(vehicle);
		entryPoint.vehicleHeading = GET_ENTITY_HEADING(vehicle);
		entryPoint.playerHeading  = GET_ENTITY_HEADING(playerPed);
		entryPoint.vehicleCoords  = GET_ENTITY_COORDS(vehicle, false);
		entryPoint.playerCoords   = GET_ENTITY_COORDS(playerPed, false);
		entryPoint.insideVehicle  = inVehicle;
	}
}

static void OnStart()
{
	if (!ComponentExists<Tracking>())
		return;

	auto vehicle              = entryPoint.vehicle;
	auto const vehicleModel   = entryPoint.vehicleModel;
	auto const vehicleCoords  = entryPoint.vehicleCoords;
	auto const playerCoords   = entryPoint.playerCoords;
	auto const playerHeading  = entryPoint.playerHeading;
	auto const vehicleHeading = entryPoint.vehicleHeading;
	auto const insideVehicle  = entryPoint.insideVehicle;

	auto const player         = PLAYER_PED_ID();
	auto const invalidCoords  = vehicleCoords.x == 0 && vehicleCoords.y == 0 && vehicleCoords.z == 0;

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

static void OnInit()
{
	if (ComponentExists<Tracking>())
		GetComponent<Tracking>()->AddTracker([]() { UpdateVehicleEntryPoint(); return true; });
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, OnInit,
	{
		.Name = "Try Again",
		.Id = "veh_try_again",
        .EffectGroupType = EffectGroupType::Teleport
	}
);

