#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static std::vector<std::uint32_t> spawnedAnimals;

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

static std::array<std::uint32_t, 4> landAnimals {
	GET_HASH_KEY("a_c_mtlion"),   GET_HASH_KEY("a_c_rottweiler"), GET_HASH_KEY("a_c_shepherd"),     GET_HASH_KEY("a_c_coyote"),
};

static void OnStart()
{
	spawnedAnimals.clear();
}

static void OnStop()
{
	for (auto const &animal : spawnedAnimals)
		if (DOES_ENTITY_EXIST(animal))
			DeleteEntity(animal);
	spawnedAnimals.clear();
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	if (!IS_PED_SHOOTING(player))
		return;

	Vector3 impactCoordinates {};
	GET_PED_LAST_WEAPON_IMPACT_COORD(player, &impactCoordinates);
	if (impactCoordinates.x == 0 && impactCoordinates.y == 0 && impactCoordinates.z == 0)
		return;

	auto constexpr maxIndex = landAnimals.size() - 1;
	auto const randomIndex  = GET_RANDOM_INT_IN_RANGE(0, maxIndex);
	auto const modelHash    = landAnimals[randomIndex];
	LoadModel(modelHash);

	auto const hostileAnimal =
	    CREATE_PED(28, modelHash, impactCoordinates.x, impactCoordinates.y, impactCoordinates.z, 0, false, false);
	spawnedAnimals.push_back(hostileAnimal);

	WAIT(200);
	CLEAR_PED_TASKS_IMMEDIATELY(hostileAnimal);
	TASK_COMBAT_PED(hostileAnimal, player, 0, 16);
	SET_PED_RELATIONSHIP_GROUP_HASH(hostileAnimal, GET_HASH_KEY("HATES_PLAYER"));
	TASK_GO_TO_ENTITY(hostileAnimal, player, -1, 2.0f, 3.0f, 0, 0);

	if (spawnedAnimals.size() > 15)
	{
		auto const oldest = spawnedAnimals.front();
		if (DOES_ENTITY_EXIST(oldest))
			DeleteEntity(oldest);
		spawnedAnimals.erase(spawnedAnimals.begin());
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Pokemon", 
        .Id = "peds_pokemon", 
        .IsTimed = true
    }
);
