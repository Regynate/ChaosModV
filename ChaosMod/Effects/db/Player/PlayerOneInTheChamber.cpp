#include "Effects/Register/RegisterEffect.h"
#include "Memory/WeaponPool.h"
#include <stdafx.h>

CHAOS_VAR std::vector<Ped> killedPeds;
CHAOS_VAR Hash goldenGunHash = GET_HASH_KEY("weapon_doubleaction");
CHAOS_VAR int goldenGunAmmo  = 1;

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

static void GiveGoldenGun()
{
	auto const player = PLAYER_PED_ID();
	GIVE_WEAPON_TO_PED(player, goldenGunHash, goldenGunAmmo, false, true);
}

static void GiveAmmoOnPedKilled()
{
	auto const player = PLAYER_PED_ID();
	for (auto const ped : GetAllPeds())
	{
		if (IS_PED_DEAD_OR_DYING(ped, false))
		{
			auto const pedSourceOfDeath = GET_PED_SOURCE_OF_DEATH(ped);
			if (pedSourceOfDeath == player)
			{
				auto it = std::find(killedPeds.begin(), killedPeds.end(), ped);
				if (it == killedPeds.end())
				{
					killedPeds.push_back(ped);

					goldenGunAmmo++;
					SET_AMMO_IN_CLIP(player, goldenGunHash, goldenGunAmmo);
				}
			}
		}
	}
}

static void CheckMissedShot()
{
	auto const player = PLAYER_PED_ID();

	if (IS_PED_SHOOTING(player))
	{
		Entity aimedEntity{};
		bool isAiming = GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(PLAYER_ID(), &aimedEntity);

		if (!isAiming || !DOES_ENTITY_EXIST(aimedEntity))
		{
			if (goldenGunAmmo > 0)
			{
				goldenGunAmmo--;
				SET_AMMO_IN_CLIP(player, goldenGunHash, goldenGunAmmo);
			}
		}
	}
}

static void OnStart()
{
	StoreWeapons();
	REMOVE_ALL_PED_WEAPONS(PLAYER_PED_ID(), false);
	GiveGoldenGun();
}

static void OnStop()
{
	RestoreWeapons();
}

static void OnTick()
{
	CheckMissedShot();
	GiveAmmoOnPedKilled();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
	{
		.Name = "One In The Chamber",
		.Id = "player_one_in_the_chamber",
		.IsTimed = true
	}
);
