#include "EntityTracking.h"

void EntityTracking::UpdateVehicleEntryPoint()
{
	auto const playerPed = PLAYER_PED_ID();

	if (!DOES_ENTITY_EXIST(playerPed))
		return;

	auto const inVehicle       = IS_PED_IN_ANY_VEHICLE(playerPed, false);
	auto const enteringVehicle = IS_PED_IN_ANY_VEHICLE(playerPed, true);

	if (!enteringVehicle)
	{
		m_LastVehicleEntryPoint.updateFlag = true;
		return;
	}

	auto const vehicle = GET_VEHICLE_PED_IS_USING(playerPed);

	if (m_LastVehicleEntryPoint.vehicle != vehicle || m_LastVehicleEntryPoint.updateFlag)
	{
		m_LastVehicleEntryPoint.updateFlag     = false;
		m_LastVehicleEntryPoint.vehicle        = vehicle;
		m_LastVehicleEntryPoint.vehicleModel   = GET_ENTITY_MODEL(vehicle);
		m_LastVehicleEntryPoint.vehicleHeading = GET_ENTITY_HEADING(vehicle);
		m_LastVehicleEntryPoint.playerHeading  = GET_ENTITY_HEADING(playerPed);
		m_LastVehicleEntryPoint.vehicleCoords  = GET_ENTITY_COORDS(vehicle, false);
		m_LastVehicleEntryPoint.playerCoords   = GET_ENTITY_COORDS(playerPed, false);
		m_LastVehicleEntryPoint.insideVehicle  = inVehicle;
        LOG("entering: " << enteringVehicle << " invehicle: " << inVehicle);
	}
}

EntityTracking::EntityTracking()
{
}

void EntityTracking::OnRun()
{
	UpdateVehicleEntryPoint();

	for (auto const &trackedEntity : m_TrackedEntities)
		trackedEntity.foo(trackedEntity.entity);
}

void EntityTracking::OnModPauseCleanup()
{
	m_TrackedEntities.clear();
}

void EntityTracking::AddTracker(Entity entity, std::function<void(Entity)> tracker, std::string id)
{
	m_TrackedEntities.emplace_back(TrackedEntity { entity, tracker, id });
}

VehicleEntryPoint EntityTracking::GetLastPlayerVehicleEntryPoint()
{
	return m_LastVehicleEntryPoint;
}
