#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>
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

static void SetPlayerModel(const Hash hash)
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

CHAOS_VAR std::array<Hash, 5> flyingAnimals { GET_HASH_KEY("a_c_chickenhawk"), GET_HASH_KEY("a_c_cormorant"),
	                                          GET_HASH_KEY("a_c_crow"), GET_HASH_KEY("a_c_pigeon"),
	                                          GET_HASH_KEY("a_c_seagull") };

CHAOS_VAR Hash previousModel {};
CHAOS_VAR bool hasDiedOrFinished { false };

static void OnStart()
{
	StoreWeapons();

	auto const oldModel      = GET_ENTITY_MODEL(PLAYER_PED_ID());
	auto const michealModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	if (oldModel != michealModel && oldModel != franklinModel && oldModel != trevorModel)
		previousModel = franklinModel;
	else
		previousModel = oldModel;

	auto const max            = flyingAnimals.size();
	auto const randomFlyingAnimal = GET_RANDOM_INT_IN_RANGE(0, max);
	SetPlayerModel(flyingAnimals[randomFlyingAnimal]);
}

static void OnStop()
{
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
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Player Becomes Flying Animal", 
        .Id = "player_becomes_flying_animal", 
        .IsTimed = true,
		.IsShortDuration = true,
        .EffectGroupType = EffectGroupType::Teleport
    }
);
