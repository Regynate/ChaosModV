#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

CHAOS_VAR std::vector<std::int32_t> spawnedPeds;
CHAOS_VAR auto constexpr maxPeds = 10;

static bool RequestControlEntity(Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;

	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(Entity entity)
{
	if (RequestControlEntity(entity))
	{
		if (!IS_ENTITY_A_MISSION_ENTITY(entity))
			SET_ENTITY_AS_MISSION_ENTITY(entity, true, true);
		DELETE_ENTITY(&entity);
	}
}

static void SpawnBodyguardAtCoords(const Vector3 coords)
{
	auto const player = PLAYER_PED_ID();

	if (spawnedPeds.size() >= maxPeds)
	{
		auto const pedToRemove = spawnedPeds.front();
		if (DOES_ENTITY_EXIST(pedToRemove))
		{
			DeleteEntity(pedToRemove);
		}
		spawnedPeds.erase(spawnedPeds.begin());
	}

	static std::array<std::uint32_t, 3> weaponList = { GET_HASH_KEY("weapon_pistol"), GET_HASH_KEY("weapon_smg"),
		                                                     GET_HASH_KEY("weapon_assaultrifle") };

	auto constexpr max                                   = weaponList.size();
	auto const randomIndex                               = GET_RANDOM_INT_IN_RANGE(0, max);
	auto const randomWeapon                              = weaponList[randomIndex];

	auto const model                                     = GET_HASH_KEY("a_m_y_business_01");

	LoadModel(model);

	auto const ped   = CreatePoolPed(4, model, coords.x, coords.y, coords.z, 0.f);

	auto const group = GET_PED_GROUP_INDEX(player);
	SET_PED_AS_GROUP_MEMBER(ped, group);
	SET_PED_COMBAT_RANGE(ped, 2);
	SET_PED_ALERTNESS(ped, 100);
	SET_PED_ACCURACY(ped, 100);
	SET_PED_CAN_SWITCH_WEAPON(ped, 1);
	SET_PED_SHOOT_RATE(ped, 200);
	SET_PED_KEEP_TASK(ped, true);
	TASK_COMBAT_HATED_TARGETS_AROUND_PED(ped, 5000.f, 0);
	GIVE_WEAPON_TO_PED(ped, randomWeapon, 100, true, true);
	SET_PED_KEEP_TASK(ped, true);
	SET_PED_GENERATES_DEAD_BODY_EVENTS(ped, true);

	spawnedPeds.emplace_back(ped);
}

static void OnStart()
{
	spawnedPeds.clear();
}

static void OnStop()
{
	for (auto const ped : spawnedPeds)
	{
		if (DOES_ENTITY_EXIST(ped))
		{
			DeleteEntity(ped);
		}
	}
	spawnedPeds.clear();
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	auto const shooting = IS_PED_SHOOTING(player);

	if (!shooting)
	{
		return;
	}

	Vector3 impactCoords{};
	GET_PED_LAST_WEAPON_IMPACT_COORD(player, &impactCoords);

	if (impactCoords.x == 0 && impactCoords.y == 0 && impactCoords.z == 0)
	{
		return;
	}

	SpawnBodyguardAtCoords(impactCoords);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Bullet Men", 
        .Id = "peds_bullet_men", 
        .IsTimed = true
    }
);
