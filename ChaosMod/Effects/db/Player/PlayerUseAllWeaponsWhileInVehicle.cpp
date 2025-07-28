#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>

#include "Memory/Weapon.h"
#include "Memory/WeaponPool.h"

CHAOS_VAR std::map<Hash, std::uint32_t> originalValues;

CHAOS_VAR const int pistolGroupHash = 0x18D5FA97;

static void OnStart()
{
	originalValues.clear();

	for (const auto &weapon : Memory::GetAllWeapons())
	{
		originalValues.emplace(weapon, Memory::GetWeaponGroup(weapon));
		Memory::SetWeaponGroup(weapon, pistolGroupHash);
	}
}

static void OnStop()
{
	for (const auto &weapon : Memory::GetAllWeapons())
		if (originalValues.contains(weapon))
			Memory::SetWeaponGroup(weapon, originalValues[weapon]);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr, 
	{
		.Name = "Unlock All Weapons In Vehicles",
		.Id = "player_use_any_weapons_while_in_vehicle",
		.IsTimed = true
	}
);