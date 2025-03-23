#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static bool RequestControlEntity(Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(Entity entity)
{
	if (!RequestControlEntity(entity))
		return;
	if (!IS_ENTITY_A_MISSION_ENTITY(entity))
		SET_ENTITY_AS_MISSION_ENTITY(entity, true, true);
	DELETE_ENTITY(&entity);
}

CHAOS_VAR std::array<Hash, 5> femaleModels { GET_HASH_KEY("a_f_m_beach_01"), GET_HASH_KEY("a_f_y_beach_01"),
	                                              GET_HASH_KEY("a_f_y_juggalo_01"), GET_HASH_KEY("a_f_y_fitness_01"),
	                                              GET_HASH_KEY("s_f_y_hooker_01") };

CHAOS_VAR std::array<Hash, 5> luxuryCars { GET_HASH_KEY("adder"), GET_HASH_KEY("t20"), GET_HASH_KEY("banshee"),
	                                            GET_HASH_KEY("zentorno"), GET_HASH_KEY("turismor") };

static void TransformPedsIntoWomen(const Ped ped)
{
	auto const player = PLAYER_PED_ID();
	if (ped == player || !DOES_ENTITY_EXIST(ped) || IS_PED_DEAD_OR_DYING(ped, true))
		return;

	auto const randomModel = femaleModels[GET_RANDOM_INT_IN_RANGE(0, femaleModels.size())];
	LoadModel(randomModel);

	auto const pedCoords = GET_ENTITY_COORDS(ped, true);
	auto const heading   = GET_ENTITY_HEADING(ped);
	auto const vehicle   = GET_VEHICLE_PED_IS_IN(ped, false);
	auto const seatIndex = GET_SEAT_PED_IS_TRYING_TO_ENTER(ped);

	DeleteEntity(ped);

	auto const spawnHeightOffset = DOES_ENTITY_EXIST(vehicle) ? 2.0f : 0.0f;
	auto const newPed =
	    CREATE_PED(26, randomModel, pedCoords.x, pedCoords.y, pedCoords.z + spawnHeightOffset, heading, true, true);

	SET_PED_RELATIONSHIP_GROUP_HASH(newPed, GET_HASH_KEY("HATES_PLAYER"));
	TASK_GO_TO_ENTITY(newPed, player, -1, 2.0f, 3.0f, 0, 0);
	SET_PED_COMBAT_ATTRIBUTES(newPed, 13, true);
	TASK_COMBAT_PED(newPed, player, 0, 16);
	SET_PED_KEEP_TASK(newPed, true);

	if (DOES_ENTITY_EXIST(vehicle))
		SET_PED_INTO_VEHICLE(newPed, vehicle, seatIndex);
}

static void TransformCarsIntoLuxury(const Vehicle vehicle)
{
	auto const player        = PLAYER_PED_ID();
	auto const playerVehicle = GET_VEHICLE_PED_IS_IN(player, false);
	if (vehicle == playerVehicle || !DOES_ENTITY_EXIST(vehicle))
		return;

	auto const randomModel = luxuryCars[GET_RANDOM_INT_IN_RANGE(0, luxuryCars.size())];
	LoadModel(randomModel);

	auto const vehicleCoords = GET_ENTITY_COORDS(vehicle, false);
	auto const heading       = GET_ENTITY_HEADING(vehicle);

	DeleteEntity(vehicle);

	auto const luxuryVehicle =
	    CREATE_VEHICLE(randomModel, vehicleCoords.x, vehicleCoords.y, vehicleCoords.z, heading, true, true, false);
	SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(luxuryVehicle, 255, 0, 255);
	SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(luxuryVehicle, 150, 0, 150);
}

static void OnStart()
{
	for (auto const ped : GetAllPeds())
		TransformPedsIntoWomen(ped);
	for (auto const vehicle : GetAllVehs())
		TransformCarsIntoLuxury(vehicle);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr,
    {
        .Name = "Womanizer",
        .Id = "peds_womanizer",
        .IsTimed = false
    }
);