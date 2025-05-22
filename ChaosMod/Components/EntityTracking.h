#pragma once

#include "Components/Component.h"

#include "../vendor/scripthookv/inc/types.h"

#include "Util/OptionsFile.h"

#include <functional>
#include <string>
#include <vector>

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
};

class EntityTracking : public Component
{
	struct TrackedEntity
	{
		std::vector<std::function<bool(Entity)>> tick;
		std::vector<std::function<void()>> cleanup;

		TrackedEntity() : tick(), cleanup()
		{
		}
	};

	std::map<Entity, TrackedEntity> m_TrackedEntities;

	VehicleEntryPoint m_LastVehicleEntryPoint;

	OptionsFile m_ConfigFile { "chaosmod/configs/tracking.ini" };

	void UpdateVehicleEntryPoint();

  public:
	EntityTracking();

	virtual void OnRun() override;
	virtual void OnModPauseCleanup() override;

	// gets called every tick; given entity is passed as an argument to tracker
	void AddTracker(Entity entity, const std::function<bool(Entity)> &tracker);

	// gets called once the entity is removed from the world
	void AddCleanupTracker(Entity entity, const std::function<void()> &tracker);

	VehicleEntryPoint GetLastPlayerVehicleEntryPoint();

	void AddPoleSpawnDriveTracker();
	void AddPoleSpawnFlyTracker();
};