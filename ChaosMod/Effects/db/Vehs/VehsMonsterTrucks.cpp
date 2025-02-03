#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

#include "Memory/Vehicle.h"

struct VehicleData
{
	float raise;
};

std::map<Hash, VehicleData> vehiclesMap;
std::map<Vehicle, Vector3> vehiclesCGMap;

static void OnStop()
{
	for (auto const &[model, data] : vehiclesMap)
	{
		LoadModel(model);
		Vehicle temp = CREATE_VEHICLE(model, 0.f, 0.f, -50.f, 0.f, true, false, true);
		FREEZE_ENTITY_POSITION(temp, true);

		Memory::SetVehicleRaise(temp, data.raise);

		DELETE_ENTITY(&temp);
		WAIT(0);
	}

	for (auto const &[veh, CG] : vehiclesCGMap)
		SET_CGOFFSET(veh, CG.x, CG.y, CG.z);

	vehiclesMap.clear();
	vehiclesCGMap.clear();
}

static void OnTick()
{
	for (Vehicle veh : GetAllVehs())
	{
		if (!DOES_ENTITY_EXIST(veh))
			continue;

		Hash vehModel = GET_ENTITY_MODEL(veh);
		if (!vehiclesMap.contains(vehModel))
		{
			vehiclesMap.emplace(vehModel, VehicleData(0.f));

			_SET_VEHICLE_WHEELS_DEAL_DAMAGE(veh, true);

			Memory::SetVehicleRaise(veh, 1.f);
		}

		if (!vehiclesCGMap.contains(veh))
		{
			Vector3 ogCG = GET_CGOFFSET(veh);
			SET_CGOFFSET(veh, ogCG.x, ogCG.y, ogCG.z - 1.2f);

			vehiclesCGMap.emplace(veh, ogCG);
		}
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick,
	{ 
		.Name = "Monster Trucks", 
		.Id = "vehs_monster_trucks", 
		.IsTimed = true 
	}
);