#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>

CHAOS_VAR auto constexpr maxBoars      = 5;
CHAOS_VAR auto constexpr spawnRadius   = 20.0f;
CHAOS_VAR auto constexpr maxDistance   = 50.0f;
CHAOS_VAR auto constexpr boarDamage    = 10;
CHAOS_VAR auto constexpr damageCooldownMs = 500;
CHAOS_VAR std::vector<Ped> spawnedBoars;
CHAOS_VAR std::unordered_map<int, int> boarBlips;

static Vector3 GetRandomSpawnLocation(const Vector3 &playerCoords)
{
    float angle = GET_RANDOM_FLOAT_IN_RANGE(0.0f, 360.0f);
    float distance = GET_RANDOM_FLOAT_IN_RANGE(5.0f, spawnRadius);

    Vector3 spawnCoords{};
    spawnCoords.x = playerCoords.x + distance * cos(angle);
    spawnCoords.y = playerCoords.y + distance * sin(angle);
    spawnCoords.z = playerCoords.z;

    return spawnCoords;
}

static void MakeBoarCharge(const Ped boar)
{
    auto const player = PLAYER_PED_ID();
    auto const playerCoords = GET_ENTITY_COORDS(player, true);
    TASK_GO_STRAIGHT_TO_COORD(boar, playerCoords.x, playerCoords.y, playerCoords.z, 6.0f, -1, 0.0f, 0.0f);
}

static void ApplyBoarDamage(const Ped boar)
{
    auto const player = PLAYER_PED_ID();
    auto const playerCoords = GET_ENTITY_COORDS(player, true);
    auto const boarCoords = GET_ENTITY_COORDS(boar, true);
    auto const distance = GET_DISTANCE_BETWEEN_COORDS(playerCoords.x, playerCoords.y, playerCoords.z, boarCoords.x, boarCoords.y, boarCoords.z, true);

    if (distance < 1.5f) {
        auto const currentHealth = GET_ENTITY_HEALTH(player);
        SET_ENTITY_HEALTH(player, currentHealth - boarDamage, 0);
		PLAY_PAIN(player, 8, 0, 0);
        WAIT(damageCooldownMs);
    }
}
static void RemoveBoar(Ped boar)
{
	if (boarBlips.count(boar))
	{
		REMOVE_BLIP(&boarBlips[boar]);
		boarBlips.erase(boar);
	}
}

static void SpawnBoar()
{
    auto const player = PLAYER_PED_ID();
    auto const playerCoords = GET_ENTITY_COORDS(player, true);
    auto const boarModel = GET_HASH_KEY("a_c_boar");
    LoadModel(boarModel);

    auto const spawnCoords = GetRandomSpawnLocation(playerCoords);
    auto const boar = CreatePoolPed(28, boarModel, spawnCoords.x, spawnCoords.y, spawnCoords.z, 0.0f);
    SET_ENTITY_AS_MISSION_ENTITY(boar, true, true);
    SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(boar, true);
    SET_PED_FLEE_ATTRIBUTES(boar, 0, true);
    MakeBoarCharge(boar);

    auto const blip = ADD_BLIP_FOR_ENTITY(boar);
    SET_BLIP_COLOUR(blip, 1);
    boarBlips[boar] = blip;

    spawnedBoars.emplace_back(boar);
}

static void ManageBoars()
{
    auto const player = PLAYER_PED_ID();
    auto const playerCoords = GET_ENTITY_COORDS(player, true);

    spawnedBoars.erase(std::remove_if(spawnedBoars.begin(), spawnedBoars.end(), [&](Ped boar) {
		auto const boarCoords = GET_ENTITY_COORDS(boar, true);
		auto const distance   = GET_DISTANCE_BETWEEN_COORDS(
            playerCoords.x, playerCoords.y, playerCoords.z, boarCoords.x,
            boarCoords.y, boarCoords.z, true);
		auto const shouldRemove = IS_PED_DEAD_OR_DYING(boar, false) || distance > maxDistance;
		

		if (shouldRemove)
		{
			RemoveBoar(boar);
			return shouldRemove;
		}

		MakeBoarCharge(boar);
		ApplyBoarDamage(boar);

		if (shouldRemove)
			RemoveBoar(boar);

		return shouldRemove;
    }), spawnedBoars.end());

    while (spawnedBoars.size() < maxBoars) {
        SpawnBoar();
    }
}
static void OnStart()
{
    spawnedBoars.clear();
    boarBlips.clear();
    while (spawnedBoars.size() < maxBoars) {
        SpawnBoar();
    }
}

static void OnStop()
{
    for (const auto &boar : spawnedBoars)
        RemoveBoar(boar);
    
    spawnedBoars.clear();
    boarBlips.clear();
}

static void OnTick()
{
    ManageBoars();
}
// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Bore Ragnarok",
        .Id = "peds_bore_ragnarok", 
        .IsTimed = true
    }
);
