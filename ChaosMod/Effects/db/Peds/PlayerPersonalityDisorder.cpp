#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Memory/WeaponPool.h"
#include <ranges>

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

CHAOS_VAR std::array<Hash, 3> characters { GET_HASH_KEY("player_zero"), GET_HASH_KEY("player_one"),
	                                             GET_HASH_KEY("player_two") };

CHAOS_VAR Hash previousModel {};

static void OnStart()
{
	auto const player        = PLAYER_PED_ID();
	auto const playerModel   = GET_ENTITY_MODEL(player);

	auto const michealModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	if (playerModel != michealModel && playerModel != franklinModel && playerModel != trevorModel)
		previousModel = franklinModel;
	else 
		previousModel = playerModel;
}

static void OnStop()
{
	Vehicle vehicle {};

	auto const player      = PLAYER_PED_ID();

	auto const isInVehicle = IS_PED_IN_ANY_VEHICLE(player, true);
	if (isInVehicle)
		vehicle = GET_VEHICLE_PED_IS_IN(player, false);

	SetPlayerModel(previousModel);

	if (vehicle)
		SET_PED_INTO_VEHICLE(PLAYER_PED_ID(), vehicle, -1);
}

static void OnTick()
{
	Vehicle vehicle{};

	auto const player            = PLAYER_PED_ID();

	auto const isInVehicle = IS_PED_IN_ANY_VEHICLE(player, true);
	if (isInVehicle)
		vehicle = GET_VEHICLE_PED_IS_IN(player, false);

	auto const max             = characters.size();
	auto const randomCharacter = GET_RANDOM_INT_IN_RANGE(0, max);

	SetPlayerModel(characters[randomCharacter]);

	if (vehicle)
		SET_PED_INTO_VEHICLE(PLAYER_PED_ID(), vehicle, -1);
	
	auto const randomWaitTime = GET_RANDOM_INT_IN_RANGE(5000, 10000);
	WAIT(randomWaitTime);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Personality Disorder", 
        .Id = "player_personality_disorder", 
        .IsTimed = true
    }
);
