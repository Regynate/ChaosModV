#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>

#include "Memory/Weapon.h"
#include "Memory/WeaponPool.h"

struct OriginalSpread
{
	float accuracySpread;
	float batchSpread;
};

CHAOS_VAR std::map<Hash, OriginalSpread> originalValues;

static void OnStart()
{
	originalValues.clear();

	for (const auto &weapon : Memory::GetAllWeapons())
	{
		originalValues.emplace(weapon, OriginalSpread { Memory::GetWeaponAccuracySpread(weapon), Memory::GetWeaponBatchSpread(weapon) });
		Memory::SetWeaponAccuracySpread(weapon, 250.f);
		Memory::SetWeaponBatchSpread(weapon, 250.f);
	}
}

static void OnStop()
{
	for (const auto &weapon : Memory::GetAllWeapons())
	{
		if (originalValues.contains(weapon))
		{
			Memory::SetWeaponAccuracySpread(weapon, originalValues[weapon].accuracySpread);
			Memory::SetWeaponBatchSpread(weapon, originalValues[weapon].batchSpread);
		}
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr, 
	{
		.Name = "High Bulletspread",
		.Id = "misc_high_bulletspread",
		.IsTimed = true
	}
);