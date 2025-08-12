#include <stdafx.h>

#include "Components/EffectDispatcher.h"
#include "Effects/Register/RegisterEffect.h"
#include "Memory/Hooks/ScriptThreadRunHook.h"
#include "Memory/Water.h"
#include "Memory/WeaponPool.h"
#include <ranges>

CHAOS_VAR std::vector<std::pair<Hash, int>> storedWeapons;

static void StoreWeapons()
{
	auto const playerPed = PLAYER_PED_ID();

	for (auto const weaponHash : Memory::GetAllWeapons())
	{
		if (HAS_PED_GOT_WEAPON(playerPed, weaponHash, false))
		{
			auto const ammo = GET_AMMO_IN_PED_WEAPON(playerPed, weaponHash);
			storedWeapons.push_back({ weaponHash, ammo });
		}
	}
}

static void RestoreWeapons()
{
	auto const playerPed = PLAYER_PED_ID();

	REMOVE_ALL_PED_WEAPONS(playerPed, false);

	for (auto const &weaponData : storedWeapons)
		GIVE_WEAPON_TO_PED(playerPed, weaponData.first, weaponData.second, false, true);
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
	auto const playerPed  = PLAYER_PED_ID();
	auto const result     = GET_RANDOM_INT_IN_RANGE(0, 100);
	auto const coords     = GET_ENTITY_COORDS(playerPed, false);
	auto const sharkModel = GET_HASH_KEY("a_c_sharktiger");
	if (result == 69)
		CreatePoolPed(28, sharkModel, coords.x, coords.y, coords.z, 0.f);
}

static void OnStart()
{
	Hooks::EnableScriptThreadBlock();
	auto const playerPed     = PLAYER_PED_ID();

	auto const oldModel      = GET_ENTITY_MODEL(playerPed);
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

	auto const swimming = IS_PED_SWIMMING(playerPed);
	if (swimming)
	{
		auto const coords = GET_ENTITY_COORDS(playerPed, false);
		SET_ENTITY_COORDS(playerPed, coords.x, coords.y, coords.z - 3.f, false, false, false, false);
		StoreWeapons();
		SetPlayerModel(selectedAnimal);
		SpawnSharkOnChance();
		return;
	}

	auto const quadCenter = Memory::GetClosestWaterQuadCenter(GET_ENTITY_COORDS(playerPed, false), 0.f);

	SET_ENTITY_COORDS(playerPed, quadCenter.x, quadCenter.y, quadCenter.z - 5.0f, false, false, false, false);

	WAIT(1000);

	StoreWeapons();
	SetPlayerModel(selectedAnimal);

	SpawnSharkOnChance();
}

static void OnStop()
{
	Hooks::DisableScriptThreadBlock();
	if (previousModel == 0)
		return;
	SetPlayerModel(previousModel);
	RestoreWeapons();
}

static void OnTick()
{
	auto const playerPed = PLAYER_PED_ID();

	if (hasDiedOrFinished)
		return;

	if (IS_PED_DEAD_OR_DYING(playerPed, false))
	{
		SetPlayerModel(previousModel);
		RestoreWeapons();
		hasDiedOrFinished = true;
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Become Sea Animal In Water", 
        .Id = "player_becomes_water_animal", 
        .IsTimed = true,
		.IsShortDuration = true,
        .EffectGroupType = EffectGroupType::Teleport
    }
);
