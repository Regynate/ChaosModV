#pragma once

#include "Components/Component.h"

#include "../vendor/scripthookv/inc/types.h"

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
		Entity entity;
		std::function<void(Entity)> foo;
		std::string id;
	};

	std::vector<TrackedEntity> m_TrackedEntities;

	VehicleEntryPoint m_LastVehicleEntryPoint;

    void UpdateVehicleEntryPoint();
  public:
	EntityTracking();

	virtual void OnRun() override;
	virtual void OnModPauseCleanup() override;

	void AddTracker(Entity entity, std::function<void(Entity)> tracker, std::string id = "");

	VehicleEntryPoint GetLastPlayerVehicleEntryPoint();
};