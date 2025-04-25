#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>

#include "Memory/Weapon.h"
#include "Memory/WeaponPool.h"

CHAOS_VAR std::map<Hash, float> originalValues;

static void OnStart()
{
	originalValues.clear();

	for (const auto &weapon : Memory::GetAllWeapons())
	{
		originalValues.emplace(weapon, Memory::GetWeaponRange(weapon));
		Memory::SetWeaponRange(weapon, 9999.f);
	}
}

static void OnStop()
{
	for (const auto &weapon : Memory::GetAllWeapons())
		if (originalValues.contains(weapon))
			Memory::SetWeaponRange(weapon, originalValues[weapon]);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr, 
	{
		.Name = "Infinite Weapon Range",
		.Id = "misc_infinite_weapon_range",
		.IsTimed = true
	}
);