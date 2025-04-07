#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Memory/WeaponPool.h"

CHAOS_VAR std::vector<std::pair<Hash, int>> storedWeapons;

static void StoreWeapons()
{
	auto const player = PLAYER_PED_ID();

	for (auto const weaponHash : Memory::GetAllWeapons())
	{
		if (HAS_PED_GOT_WEAPON(player, weaponHash, false))
		{
			auto const ammo = GET_AMMO_IN_PED_WEAPON(player, weaponHash);
			storedWeapons.push_back({ weaponHash, ammo });
		}
	}
}

static void RestoreWeapons()
{
	auto const player = PLAYER_PED_ID();

	REMOVE_ALL_PED_WEAPONS(player, false);

	for (auto const &weaponData : storedWeapons)
		GIVE_WEAPON_TO_PED(player, weaponData.first, weaponData.second, false, true);
}

static void SetPlayerModel(const std::uint32_t hash)
{
	if (!IS_MODEL_IN_CDIMAGE(hash))
		return;
	if (!IS_MODEL_VALID(hash))
		return;
	while (!HAS_MODEL_LOADED(hash))
	{
		REQUEST_MODEL(hash);
		WAIT(0);
	}

	StoreWeapons();
	SET_PLAYER_MODEL(PLAYER_ID(), hash);
	RestoreWeapons();
}

static bool RequestControlEntity(Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

CHAOS_VAR Hash previousModel {};
CHAOS_VAR bool hasSpawned = false;

static void StorePlayerModel()
{
	auto const player        = PLAYER_PED_ID();
	auto const playerModel   = GET_ENTITY_MODEL(player);
	auto const michealModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	auto const isPlayerModelNotACharacter =
	    playerModel != michealModel && playerModel != franklinModel && playerModel != trevorModel;

	if (isPlayerModelNotACharacter)
		previousModel = franklinModel;
	else
		previousModel = playerModel;
}

static void OnStart()
{
	StorePlayerModel();

	auto const alienHash = GET_HASH_KEY("s_m_m_movalien_01");
	LoadModel(alienHash);
	SetPlayerModel(alienHash);

	auto const agent1Hash = GET_HASH_KEY("s_m_m_highsec_02");
	auto const agent2Hash = GET_HASH_KEY("s_m_m_highsec_01");
	auto const agentCar   = GET_HASH_KEY("virgo");

	LoadModel(agent1Hash);
	LoadModel(agent2Hash);
	LoadModel(agentCar);

	auto const player         = PLAYER_PED_ID();
	auto const playersVehicle = GET_VEHICLE_PED_IS_IN(player, false);
	auto const coords         = GET_ENTITY_COORDS(player, false);
	auto const heading        = GET_ENTITY_HEADING(player);

	auto const agent1     = CREATE_PED(6, agent1Hash, coords.x + 2, coords.y + 2, coords.z + 3, heading, false, false);
	auto const agent2     = CREATE_PED(6, agent2Hash, coords.x + 2, coords.y + 2, coords.z + 3, heading, false, false);
	auto const spawnedCar = CREATE_VEHICLE(agentCar, coords.x, coords.y, coords.z + 3, heading, false, false, false);

	SET_PED_INTO_VEHICLE(agent1, spawnedCar, -1);
	SET_PED_INTO_VEHICLE(agent2, spawnedCar, 0);

	GIVE_DELAYED_WEAPON_TO_PED(agent1, GET_HASH_KEY("weapon_pistol"), 100, true);

	SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(spawnedCar, 0, 0, 0);
	SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(spawnedCar, 0, 0, 0);

	constexpr auto FIRING_PATTERN_FULL_AUTO = 0xC6EE6B4C;
	TASK_DRIVE_BY(agent1, player, playersVehicle, coords.x, coords.y, coords.z, 100.f, 100, false,
	              FIRING_PATTERN_FULL_AUTO);
	TASK_DRIVE_BY(agent2, player, playersVehicle, coords.x, coords.y, coords.z, 100.f, 100, false,
	              FIRING_PATTERN_FULL_AUTO);

	SET_PED_KEEP_TASK(agent1, true);
	SET_PED_KEEP_TASK(agent2, true);
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	if (IS_PED_DEAD_OR_DYING(player, false))
	{
		WAIT(2000);
		SetPlayerModel(previousModel);
		WAIT(15000);
	}
}

static void OnStop()
{
	SetPlayerModel(previousModel);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Men In Black", 
        .Id = "peds_men_in_black", 
        .IsTimed = true,
		.IsShortDuration = true
    }
);