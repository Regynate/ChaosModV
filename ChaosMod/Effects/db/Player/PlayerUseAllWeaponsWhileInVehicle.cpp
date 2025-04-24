#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>

#include "Memory/WeaponPool.h"

CHAOS_VAR std::vector<std::uint32_t> originalValues;

CHAOS_VAR const int bulletspreadOffset = 0x5C;
CHAOS_VAR const int pistolGroupHash   = 0x18D5FA97;

static int *GetWeaponBulletspread(DWORD64 weapon)
{
	return reinterpret_cast<int *>(weapon + bulletspreadOffset);
}

static void OnStart()
{
	const auto weapons = Memory::GetAllWeaponPointers();

	originalValues.clear();

	for (size_t i = 0; i < weapons->count; i++)
	{
		const auto weapon = weapons->elements[i];
		originalValues.push_back(*GetWeaponBulletspread(weapon));
		*GetWeaponBulletspread(weapon) = pistolGroupHash;
	}
}

static void OnStop()
{
	const auto weapons = Memory::GetAllWeaponPointers();

	for (size_t i = 0; i < weapons->count; i++)
	{
		const auto weapon       = weapons->elements[i];
		*GetWeaponBulletspread(weapon) = originalValues[i];
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr, 
	{
		.Name = "Use Any Weapons While In Vehicles",
		.Id = "player_use_any_weapons_while_in_vehicle",
		.IsTimed = true
	}
);