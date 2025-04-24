#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>

#include "Memory/WeaponPool.h"

CHAOS_VAR std::vector<std::uint32_t> OriginalWeaponGroups;

CHAOS_VAR const int weaponGroupOffset = 0x5C;
CHAOS_VAR const int pistolGroupHash   = 0x18D5FA97;

static int *GetWeaponGroup(DWORD64 weapon)
{
	return reinterpret_cast<int *>(weapon + weaponGroupOffset);
}

static void OnStart()
{
	const auto weapons = Memory::GetAllWeaponPointers();

	OriginalWeaponGroups.clear();

	for (size_t i = 0; i < weapons->count; i++)
	{
		const auto weapon = weapons->elements[i];
		OriginalWeaponGroups.push_back(*GetWeaponGroup(weapon));
		*GetWeaponGroup(weapon) = pistolGroupHash;
	}
}

static void OnStop()
{
	const auto weapons = Memory::GetAllWeaponPointers();

	for (size_t i = 0; i < weapons->count; i++)
	{
		const auto weapon       = weapons->elements[i];
		*GetWeaponGroup(weapon) = OriginalWeaponGroups[i];
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