#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

CHAOS_VAR Vehicle spawnedBus = 0;
CHAOS_VAR int fadeR = 255, fadeG = 0, fadeB = 0;

static void SpawnMagicSchoolBus()
{
	auto const player         = PLAYER_PED_ID();
	auto const playerCoords   = GET_ENTITY_COORDS(player, true);
	auto const randomBusModel = GET_HASH_KEY("pbus");
	auto const jimmyModel     = GET_HASH_KEY("ig_jimmydisanto");

	LoadModel(randomBusModel);
	LoadModel(jimmyModel);

	spawnedBus = CreatePoolVehicle(randomBusModel, playerCoords.x, playerCoords.y, playerCoords.z, 0.0f);
	SET_ENTITY_AS_MISSION_ENTITY(spawnedBus, true, true);
	SET_VEHICLE_ON_GROUND_PROPERLY(spawnedBus, 0);
	SET_PED_INTO_VEHICLE(player, spawnedBus, -1);

	auto const maxSeats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(randomBusModel);
	for (auto seat = 0; seat < maxSeats; seat++)
	{
		if (IS_VEHICLE_SEAT_FREE(spawnedBus, seat, false))
		{
			auto const ped = CreatePoolPedInsideVehicle(spawnedBus, 26, jimmyModel, seat);
		}
	}
}

static void UpdateBusRGB()
{
	if (!DOES_ENTITY_EXIST(spawnedBus))
		return;

	if (fadeR > 0 && fadeB == 0)
	{
		fadeR--;
		fadeG++;
	}
	if (fadeG > 0 && fadeR == 0)
	{
		fadeG--;
		fadeB++;
	}
	if (fadeB > 0 && fadeG == 0)
	{
		fadeR++;
		fadeB--;
	}

	SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(spawnedBus, fadeR, fadeG, fadeB);
	SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(spawnedBus, fadeR, fadeG, fadeB);
}

static void OnStart()
{
	SpawnMagicSchoolBus();
}

static void OnTick()
{
	UpdateBusRGB();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, OnTick, 
    {
        .Name = "Please Let This Be A Normal Field Trip", 
        .Id = "misc_please_let_this_be_a_normal_field_trip", 
        .IsTimed = true
    }
);
