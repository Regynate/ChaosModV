#include "EntityTracking.h"

#include "Components/EffectDispatcher.h"

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
	}
}

EntityTracking::EntityTracking()
{
	if (m_ConfigFile.ReadValue<bool>("PLD", false))
		AddPoleSpawnDriveTracker();
	if (m_ConfigFile.ReadValue<bool>("PLF", false))
		AddPoleSpawnFlyTracker();
}

void EntityTracking::AddPoleSpawnDriveTracker()
{
	m_ConfigFile.SetValue("PLD", true);
	m_ConfigFile.WriteFile();

	const auto foo = [this](Entity)
	{
		static bool init     = true;
		const auto playerPed = PLAYER_PED_ID();

		if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
		{
			const auto vehicle = GET_VEHICLE_PED_IS_IN(playerPed, false);
			if (GET_ENTITY_SPEED(vehicle) > 35.f)
			{
				if (!init)
				{
					if (ComponentExists<EffectDispatcher>())
					{
						GetComponent<EffectDispatcher>()->DispatchEffect(EffectIdentifier("misc_pole_drivefast"),
						                                                 EffectDispatcher::DispatchEffectFlag_None, "",
						                                                 "spawn");
						m_ConfigFile.SetValue("PLD", false);
						m_ConfigFile.WriteFile();
						return false;
					}
				}
			}
			else
				init = false;
		}
		else
			init = false;

		return true;
	};

	AddTracker(0, foo);
}

void EntityTracking::AddPoleSpawnFlyTracker()
{
	m_ConfigFile.SetValue("PLF", true);
	m_ConfigFile.WriteFile();

	const auto foo = [this](Entity)
	{
		static bool init     = true;
		const auto playerPed = PLAYER_PED_ID();

		if (IS_PED_IN_ANY_PLANE(playerPed))
		{
			const auto vehicle = GET_VEHICLE_PED_IS_IN(playerPed, false);
			if (GET_ENTITY_SPEED(vehicle) > 30.f && GET_ENTITY_HEIGHT_ABOVE_GROUND(vehicle) > 10.f)
			{
				if (!init)
				{
					if (ComponentExists<EffectDispatcher>())
					{
						GetComponent<EffectDispatcher>()->DispatchEffect(
						    EffectIdentifier("misc_pole_fly"), EffectDispatcher::DispatchEffectFlag_None, "", "spawn");
						m_ConfigFile.SetValue("PLF", false);
						m_ConfigFile.WriteFile();
						return false;
					}
				}
			}
			else
				init = false;
		}
		else
			init = false;

		return true;
	};

	AddTracker(0, foo);
}

void EntityTracking::OnRun()
{
	UpdateVehicleEntryPoint();

	for (auto it = m_TrackedEntities.begin(); it != m_TrackedEntities.end();)
	{
		auto const &[entity, trackedEntity] = *it;
		bool keep                           = true;
		if (entity != 0 && !DOES_ENTITY_EXIST(entity))
			keep = false;
		else
			for (auto const &tick : trackedEntity.tick)
				keep = keep && tick(entity);

		if (!keep)
			for (auto const &cleanup : trackedEntity.cleanup)
				cleanup();

		if (keep)
			++it;
		else
			it = m_TrackedEntities.erase(it);
	}
}

void EntityTracking::OnModPauseCleanup()
{
	for (const auto &[_, trackedEntity] : m_TrackedEntities)
		for (auto const &cleanup : trackedEntity.cleanup)
				cleanup();
	m_TrackedEntities.clear();
}

void EntityTracking::AddTracker(Entity entity, const std::function<bool(Entity)> &tracker)
{
	m_TrackedEntities[entity].tick.push_back(tracker);
}

void EntityTracking::AddCleanupTracker(Entity entity, const std::function<void()> &tracker)
{
	m_TrackedEntities[entity].cleanup.push_back(tracker);
}

VehicleEntryPoint EntityTracking::GetLastPlayerVehicleEntryPoint()
{
	return m_LastVehicleEntryPoint;
}
