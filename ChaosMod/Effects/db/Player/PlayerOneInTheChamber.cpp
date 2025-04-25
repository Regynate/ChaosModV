#include "Effects/Register/RegisterEffect.h"
#include "Memory/WeaponPool.h"
#include <stdafx.h>

CHAOS_VAR constexpr Hash goldenGunHash = GET_HASH_KEY("weapon_doubleaction");

CHAOS_VAR std::vector<std::pair<Hash, int>> storedWeapons;

CHAOS_VAR bool hasBullet = true;

static void StoreWeapons()
{
	auto const playerPed = PLAYER_PED_ID();

	for (auto const &weaponHash : Memory::GetAllWeapons())
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

	for (auto const &[weapon, ammo] : storedWeapons)
	{
		GIVE_WEAPON_TO_PED(playerPed, weapon, ammo, false, false);
		SET_PED_AMMO(playerPed, weapon, ammo, false);
	}
}

static void GiveGoldenGun(bool forceInHand)
{
	auto const playerPed = PLAYER_PED_ID();
	if (!HAS_PED_GOT_WEAPON(playerPed, goldenGunHash, false))
	{
		GIVE_WEAPON_TO_PED(playerPed, goldenGunHash, 0, false, forceInHand);
		SET_PED_AMMO(playerPed, goldenGunHash, hasBullet ? 1 : 0, false);
	}
}

static void CheckPedDamage()
{
	auto const playerPed = PLAYER_PED_ID();
	for (auto const ped : GetAllPeds())
	{
		if (HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY(ped, playerPed, true)
		    && HAS_ENTITY_BEEN_DAMAGED_BY_WEAPON(ped, goldenGunHash, 0))
		{
			SET_ENTITY_HEALTH(ped, 0, 0);
			CLEAR_ENTITY_LAST_DAMAGE_ENTITY(ped);

			// workaround the rapid fire bug
			WAIT(1000);

			GIVE_WEAPON_TO_PED(playerPed, goldenGunHash, 0, false, true);
			SET_PED_AMMO(playerPed, goldenGunHash, 1, false);
			SET_AMMO_IN_CLIP(playerPed, goldenGunHash, 0);
		}
	}

	if (GET_AMMO_IN_PED_WEAPON(playerPed, goldenGunHash) == 0)
		hasBullet = false;
}

static void RemoveWeapons()
{
	auto const playerPed = PLAYER_PED_ID();

	for (const auto &weaponHash : Memory::GetAllWeapons())
		if (HAS_PED_GOT_WEAPON(playerPed, weaponHash, false) && weaponHash != goldenGunHash)
			REMOVE_WEAPON_FROM_PED(playerPed, weaponHash);
}

static void OnStart()
{
	hasBullet = true;
	StoreWeapons();
	REMOVE_ALL_PED_WEAPONS(PLAYER_PED_ID(), false);
	GiveGoldenGun(true);
}

static void OnStop()
{
	RestoreWeapons();
}

static void OnTick()
{
	GiveGoldenGun(false);
	CheckPedDamage();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
	{
		.Name = "One In The Chamber",
		.Id = "player_one_in_the_chamber",
		.IsTimed = true
	}
);
