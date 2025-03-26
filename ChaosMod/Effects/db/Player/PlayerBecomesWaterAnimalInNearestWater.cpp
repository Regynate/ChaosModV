#include "Components/EffectDispatcher.h"
#include "Effects/Register/RegisterEffect.h"
#include <ranges>
#include <stdafx.h>
#include "Util/LuaAPI.h"
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

	SET_PLAYER_MODEL(PLAYER_ID(), hash);
}

CHAOS_VAR Hash previousModel {};
CHAOS_VAR bool hasDiedOrFinished { false };

static void SpawnSharkOnChance()
{
	auto const player = PLAYER_PED_ID();
	auto const result = GET_RANDOM_INT_IN_RANGE(0, 100);
	auto const coords     = GET_ENTITY_COORDS(player, false);
	auto const sharkModel = GET_HASH_KEY("a_c_sharktiger");
	if (result == 69)
		CreatePoolPed(28, sharkModel, coords.x, coords.y, coords.z, 0.f);
	
}

static void OnStart()
{
	auto const player        = PLAYER_PED_ID();

	auto const oldModel      = GET_ENTITY_MODEL(player);
	auto const michealModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	if (oldModel != michealModel && oldModel != franklinModel && oldModel != trevorModel)
		previousModel = franklinModel;
	else
		previousModel = oldModel;

	static std::array<Hash, 6> waterAnimals { GET_HASH_KEY("a_c_dolphin"),     GET_HASH_KEY("a_c_fish"),
		                                      GET_HASH_KEY("a_c_sharkhammer"), GET_HASH_KEY("a_c_humpback"),
		                                      GET_HASH_KEY("a_c_killerwhale"), GET_HASH_KEY("a_c_stingray") };

	auto constexpr max           = waterAnimals.size();
	auto const randomWaterAnimal = GET_RANDOM_INT_IN_RANGE(0, max);
	auto const selectedAnimal    = waterAnimals[randomWaterAnimal];

	LoadModel(selectedAnimal);

	auto const swimming         = IS_PED_SWIMMING(player);
	if (swimming)
	{
		auto const coords = GET_ENTITY_COORDS(player, false);
		SET_ENTITY_COORDS(player, coords.x, coords.y, coords.z -3.f, false, false, false, false);
		StoreWeapons();
		SetPlayerModel(selectedAnimal);
		SpawnSharkOnChance();
		return;
	}

	auto const nearestWaterQuad = GetClosestQuad();

	for (auto const _ : std::ranges::iota_view { 0, 5 })
	{
		SET_ENTITY_COORDS(player, nearestWaterQuad.x, nearestWaterQuad.y, nearestWaterQuad.z - 5.0f, false, false,
		                  false, false);
		WAIT(0);
	}

	while (!IS_ENTITY_IN_WATER(player))
		WAIT(500);

	while (!IS_PED_SWIMMING(player))
		WAIT(500);
	StoreWeapons();
	SetPlayerModel(selectedAnimal);

	SpawnSharkOnChance();
}

static void OnStop()
{
	if (previousModel == 0)
		return;
	SetPlayerModel(previousModel);
	RestoreWeapons();
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();

	if (hasDiedOrFinished)
		return;

	if (IS_PED_DEAD_OR_DYING(player, false))
	{

		SetPlayerModel(previousModel);
		RestoreWeapons();

		hasDiedOrFinished = true;

		WAIT(10000);
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Player Becomes Water Animal In Nearest Water", 
        .Id = "player_becomes_water_animal", 
        .IsTimed = true,
		.IsShortDuration = true
    }
);
