#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Util/Vehicle.h"

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

static Ped TransformPedIntoWomen(const Ped ped)
{
	auto const player = PLAYER_PED_ID();
	if (ped == player || !DOES_ENTITY_EXIST(ped) || IS_PED_DEAD_OR_DYING(ped, true))
		return 0;

	auto const pedCoords    = GET_ENTITY_COORDS(ped, true);
	auto const playerCoords = GET_ENTITY_COORDS(player, false);
	auto const distance     = GET_DISTANCE_BETWEEN_COORDS(pedCoords.x, pedCoords.y, pedCoords.z, playerCoords.x,
	                                                      playerCoords.y, playerCoords.z, false);

	if (distance > 100)
		return 0;

	auto const randomModel       = femaleModels[GET_RANDOM_INT_IN_RANGE(0, femaleModels.size())];

	auto const heading           = GET_ENTITY_HEADING(ped);
	auto const vehicle           = GET_VEHICLE_PED_IS_IN(ped, false);

	auto const spawnHeightOffset = IS_PED_IN_ANY_VEHICLE(ped, false) ? 2.0f : 0.0f;

	DeleteEntity(ped);
	auto const newPed =
	    CreatePoolPed(26, randomModel, pedCoords.x, pedCoords.y, pedCoords.z + spawnHeightOffset, heading);

	SET_PED_RELATIONSHIP_GROUP_HASH(newPed, GET_HASH_KEY("HATES_PLAYER"));
	TASK_GO_TO_ENTITY(newPed, player, -1, 2.0f, 3.0f, 0, 0);
	SET_PED_COMBAT_ATTRIBUTES(newPed, 13, true);
	TASK_COMBAT_PED(newPed, player, 0, 16);
	SET_PED_KEEP_TASK(newPed, true);

	auto const numberOfSeats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(GET_ENTITY_MODEL(vehicle));
	for (int i = -1; i < numberOfSeats - 1; i++)
	{
		if (IS_VEHICLE_SEAT_FREE(vehicle, i, false))
		{
			SET_PED_INTO_VEHICLE(newPed, vehicle, i);
			if (IS_PED_IN_ANY_VEHICLE(newPed, false))
				return 0;
		}
	}

	return newPed;
}

static Vehicle TransformCarsIntoLuxury(const Vehicle vehicle)
{
	auto const player = PLAYER_PED_ID();
	if (IS_PED_IN_VEHICLE(player, vehicle, false) || !DOES_ENTITY_EXIST(vehicle))
		return 0;

	auto const randomModel   = luxuryCars[GET_RANDOM_INT_IN_RANGE(0, luxuryCars.size() - 1)];

	auto const luxuryVehicle = ReplaceVehicleWithModel(vehicle, randomModel, true, false);
	SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(luxuryVehicle, 255, 0, 255);
	SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(luxuryVehicle, 150, 0, 150);

	return luxuryVehicle;
}

static void OnStart()
{
	std::set<Entity> replacedEntities;

	int counter = 5;

	for (auto const vehicle : GetAllVehs())
	{
		if (!replacedEntities.contains(vehicle))
		{
			const auto newVeh = TransformCarsIntoLuxury(vehicle);
			replacedEntities.emplace(newVeh);
		}
		if (counter-- <= 0)
		{
			WAIT(0);
			counter = 5;
		}
	}
	
	for (auto const ped : GetAllPeds())
	{
		if (!replacedEntities.contains(ped))
		{
			const auto newPed = TransformPedIntoWomen(ped);
			replacedEntities.emplace(newPed);
		}
		if (counter-- <= 0)
		{
			WAIT(0);
			counter = 5;
		}
	}
}
// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr,
    {
        .Name = "Womanizer",
        .Id = "peds_womanizer",
        .IsTimed = false
    }
);