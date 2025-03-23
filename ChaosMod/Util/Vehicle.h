#pragma once

#include "Util/Natives.h"
#include "Util/Random.h"

#include "Memory/Hooks/HandleToEntityStructHook.h"
#include "Memory/Hooks/ScriptThreadRunHook.h"
#include "Memory/Vehicle.h"

#include "Util/EntityIterator.h"
#include "Util/Model.h"
#include "Util/PoolSpawner.h"

inline Vehicle CreateTempVehicle(Hash model, float x, float y, float z, float heading)
{
	LoadModel(model);

	auto veh = CREATE_VEHICLE(model, x, y, z, heading, true, false, false);
	SET_MODEL_AS_NO_LONGER_NEEDED(model);

	auto dummy = veh;
	SET_VEHICLE_AS_NO_LONGER_NEEDED(&dummy);

	return veh;
}

inline Vehicle CreateTempVehicleOnPlayerPos(Hash model, float heading)
{
	LoadModel(model);

	auto playerPos = GET_ENTITY_COORDS(PLAYER_PED_ID(), false);

	auto veh       = CREATE_VEHICLE(model, playerPos.x, playerPos.y, playerPos.z, heading, true, false, false);
	SET_MODEL_AS_NO_LONGER_NEEDED(model);

	auto dummy = veh;
	SET_VEHICLE_AS_NO_LONGER_NEEDED(&dummy);

	return veh;
}

inline void SetSurroundingPedsInVehicles(Hash vehicleHash, float maxDistance)
{
	Ped playerPed     = PLAYER_PED_ID();
	Vector3 playerPos = GET_ENTITY_COORDS(playerPed, true);

	for (Ped ped : GetAllPeds())
	{
		if (!IS_PED_A_PLAYER(ped) && !IS_PED_DEAD_OR_DYING(ped, false))
		{
			Vector3 pedPos = GET_ENTITY_COORDS(ped, true);
			// check if player is far away from entity
			if (maxDistance <= 0 || pedPos.DistanceTo(playerPos) <= maxDistance)
			{
				if (IS_PED_IN_ANY_VEHICLE(ped, false))
				{
					Vehicle veh = GET_VEHICLE_PED_IS_IN(ped, false);

					if (GET_ENTITY_MODEL(veh) == vehicleHash)
						continue;
				}

				float pedHeading = GET_ENTITY_HEADING(ped);
				Vector3 vel      = GET_ENTITY_VELOCITY(ped);

				SET_ENTITY_COORDS(ped, pedPos.x, pedPos.y, pedPos.z + 10.f, false, false, false, false);

				SET_PED_COMBAT_ATTRIBUTES(ped, 3, false); // Don't allow them to leave vehicle by themselves

				SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, true);

				Vehicle veh = CreateTempVehicle(vehicleHash, pedPos.x, pedPos.y, pedPos.z, pedHeading);
				SET_PED_INTO_VEHICLE(ped, veh, -1);
				SET_VEHICLE_ENGINE_ON(veh, true, true, true);
				SET_ENTITY_VELOCITY(veh, vel.x, vel.y, vel.z);

				TASK_VEHICLE_MISSION_PED_TARGET(ped, veh, playerPed, 13, 9999.f, 4176732, .0f, .0f, false);
			}
		}
	}
}

struct SeatPed
{
	Ped Ped;
	int SeatIndex;
};

inline Vehicle CreateVehicleWithPeds(Hash model, Vehicle oldHandle, const std::vector<SeatPed> &seatPeds,
                                     bool addToPool, const Vector3 &coords, float heading, bool engineRunning,
                                     const Vector3 &velocity, float forwardSpeed)
{
	if (!IS_MODEL_VALID(model))
		return oldHandle;

	int numberOfSeats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(model);

	Vehicle newVehicle;
	if (addToPool)
	{
		newVehicle = CreatePoolVehicle(model, coords.x, coords.y, coords.z, heading);
	}
	else
	{
		LoadModel(model);
		newVehicle = CREATE_VEHICLE(model, coords.x, coords.y, coords.z, heading, true, true, true);
		SET_MODEL_AS_NO_LONGER_NEEDED(model);

		SET_ENTITY_AS_MISSION_ENTITY(newVehicle, false, true);
	}

	for (size_t i = 0; i < seatPeds.size(); i++)
	{
		SeatPed seatPed = seatPeds.at(i);
		int seatIndex   = seatPed.SeatIndex;
		if (seatIndex >= numberOfSeats || !IS_VEHICLE_SEAT_FREE(newVehicle, seatIndex, 0))
			seatIndex = -2;

		SET_PED_INTO_VEHICLE(seatPed.Ped, newVehicle, seatIndex);
	}

	if (engineRunning)
		SET_VEHICLE_ENGINE_ON(newVehicle, true, true, false);

	SET_ENTITY_VELOCITY(newVehicle, velocity.x, velocity.y, velocity.z);
	SET_VEHICLE_FORWARD_SPEED(newVehicle, forwardSpeed);

	if (oldHandle)
	{
		bool shouldUseHook = IS_ENTITY_A_MISSION_ENTITY(oldHandle);
		Entity copy        = oldHandle;
		SET_ENTITY_AS_MISSION_ENTITY(copy, true, true);

		if (shouldUseHook)
			Hooks::EnableScriptThreadBlock();
		DELETE_VEHICLE(&copy);
		if (shouldUseHook)
		{
			Hooks::ProxyEntityHandle(oldHandle, newVehicle);
			Hooks::DisableScriptThreadBlock();
		}
	}

	// Also apply random upgrades
	SET_VEHICLE_MOD_KIT(newVehicle, 0);

	SET_VEHICLE_WHEEL_TYPE(newVehicle, g_RandomNoDeterm.GetRandomInt(0, 12));

	for (int i = 0; i < 50; i++)
	{
		int max = GET_NUM_VEHICLE_MODS(newVehicle, i);
		if (max > 0)
		{
			SET_VEHICLE_MOD(newVehicle, i, g_RandomNoDeterm.GetRandomInt(0, max - 1),
			                g_RandomNoDeterm.GetRandomInt(0, 1));
		}

		TOGGLE_VEHICLE_MOD(newVehicle, i, g_RandomNoDeterm.GetRandomInt(0, 1));
	}

	SET_VEHICLE_TYRES_CAN_BURST(newVehicle, g_RandomNoDeterm.GetRandomInt(0, 1));
	SET_VEHICLE_WINDOW_TINT(newVehicle, g_RandomNoDeterm.GetRandomInt(0, 6));

	SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(newVehicle, g_RandomNoDeterm.GetRandomInt(0, 255),
	                                  g_RandomNoDeterm.GetRandomInt(0, 255), g_RandomNoDeterm.GetRandomInt(0, 255));
	SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(newVehicle, g_RandomNoDeterm.GetRandomInt(0, 255),
	                                    g_RandomNoDeterm.GetRandomInt(0, 255), g_RandomNoDeterm.GetRandomInt(0, 255));

	_SET_VEHICLE_NEON_LIGHTS_COLOUR(newVehicle, g_RandomNoDeterm.GetRandomInt(0, 255),
	                                g_RandomNoDeterm.GetRandomInt(0, 255), g_RandomNoDeterm.GetRandomInt(0, 255));
	for (int i = 0; i < 4; i++)
		_SET_VEHICLE_NEON_LIGHT_ENABLED(newVehicle, i, true);

	_SET_VEHICLE_XENON_LIGHTS_COLOR(newVehicle, g_RandomNoDeterm.GetRandomInt(0, 12));

	return newVehicle;
}

inline Vehicle CreateRandomVehicleWithPeds(Vehicle oldHandle, const std::vector<SeatPed> &seatPeds, bool addToPool,
                                           const Vector3 &coords, float heading, bool engineRunning,
                                           const Vector3 &velocity, float forwardSpeed)
{
	static const std::vector<Hash> &vehicleModels = Memory::GetAllVehModels();
	if (vehicleModels.empty())
		return oldHandle;

	Hash newVehModel = 0;
	do
	{
		newVehModel = vehicleModels[g_RandomNoDeterm.GetRandomInt(0, vehicleModels.size() - 1)];
	} while (static_cast<size_t>(GET_VEHICLE_MODEL_NUMBER_OF_SEATS(newVehModel)) < seatPeds.size()
	         || IS_THIS_MODEL_A_TRAIN(newVehModel) || GET_VEHICLE_MODEL_ACCELERATION(newVehModel) <= 0);

	if (!newVehModel)
		return oldHandle;

	return CreateVehicleWithPeds(newVehModel, oldHandle, seatPeds, addToPool, coords, heading, engineRunning, velocity,
	                             forwardSpeed);
}

inline Vehicle ReplaceVehicleWithModel(Vehicle veh, Hash model, bool addToPool)
{
	std::vector<SeatPed> vehPeds;
	int numberOfSeats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(GET_ENTITY_MODEL(veh));
	for (int i = -1; i < numberOfSeats - 1; i++)
	{
		if (!IS_VEHICLE_SEAT_FREE(veh, i, false))
		{
			Ped ped         = GET_PED_IN_VEHICLE_SEAT(veh, i, false);
			SeatPed seatPed = { ped, i };
			vehPeds.push_back(seatPed);
		}
	}

	float heading      = GET_ENTITY_HEADING(veh);
	Vector3 vehCoords  = GET_ENTITY_COORDS(veh, 0);
	bool engineRunning = GET_IS_VEHICLE_ENGINE_RUNNING(veh);
	Vector3 velocity   = GET_ENTITY_VELOCITY(veh);
	float forwardSpeed = GET_ENTITY_SPEED(veh);

	return CreateVehicleWithPeds(model, veh, vehPeds, addToPool, vehCoords, heading, engineRunning, velocity,
	                                   forwardSpeed);
}

inline Vehicle ReplaceVehicle(Vehicle veh, bool addToPool)
{
	std::vector<SeatPed> vehPeds;
	int numberOfSeats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(GET_ENTITY_MODEL(veh));
	for (int i = -1; i < numberOfSeats - 1; i++)
	{
		if (!IS_VEHICLE_SEAT_FREE(veh, i, false))
		{
			Ped ped         = GET_PED_IN_VEHICLE_SEAT(veh, i, false);
			SeatPed seatPed = { ped, i };
			vehPeds.push_back(seatPed);
		}
	}

	float heading      = GET_ENTITY_HEADING(veh);
	Vector3 vehCoords  = GET_ENTITY_COORDS(veh, 0);
	bool engineRunning = GET_IS_VEHICLE_ENGINE_RUNNING(veh);
	Vector3 velocity   = GET_ENTITY_VELOCITY(veh);
	float forwardSpeed = GET_ENTITY_SPEED(veh);

	return CreateRandomVehicleWithPeds(veh, vehPeds, addToPool, vehCoords, heading, engineRunning, velocity,
	                                   forwardSpeed);
}

inline void DeleteVehicle(Vehicle vehicle)
{
	std::vector<Ped> vehPeds;

	Hash vehModel = GET_ENTITY_MODEL(vehicle);
	int maxSeats  = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(vehModel);
	for (int i = -1; i < maxSeats - 1; i++)
	{
		if (IS_VEHICLE_SEAT_FREE(vehicle, i, false))
			continue;

		Ped ped = GET_PED_IN_VEHICLE_SEAT(vehicle, i, false);

		CLEAR_PED_TASKS_IMMEDIATELY(ped);

		SET_PED_TO_RAGDOLL(ped, 5000, 5000, 0, true, true, false);

		vehPeds.push_back(ped);
	}

	Vector3 vehVel = GET_ENTITY_VELOCITY(vehicle);

	SET_ENTITY_AS_MISSION_ENTITY(vehicle, true, true);
	DELETE_VEHICLE(&vehicle);

	WAIT(0);

	for (Ped ped : vehPeds)
		SET_ENTITY_VELOCITY(ped, vehVel.x, vehVel.y, vehVel.z);
}

struct TryAgainData
{
	Vehicle vehicle;
	float vehicleHeading;
	float playerHeading;
	Vector3 vehicleCoords;
	Vector3 playerCoords;
};

inline TryAgainData GetLastVehicleEntryCoords()
{
	static TryAgainData data{};

	auto const player    = PLAYER_PED_ID();
	auto const inVehicle = IS_PED_IN_ANY_VEHICLE(player, true);
	auto const vehicle   = GET_VEHICLE_PED_IS_IN(player, true);

	if (!inVehicle || data.vehicle == vehicle)
		return data;

	data.vehicle        = vehicle;
	data.vehicleHeading = GET_ENTITY_HEADING(vehicle);
	data.playerHeading  = GET_ENTITY_HEADING(player);
	data.vehicleCoords  = GET_ENTITY_COORDS(vehicle, false);
	data.playerCoords   = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(player, -2.f, 2.f, 0.f);

	return data;
}