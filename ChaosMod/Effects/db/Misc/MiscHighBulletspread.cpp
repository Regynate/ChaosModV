#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>

#include "Memory/WeaponPool.h"

struct OriginalSpread
{
    float accuracySpread;
    float batchSpread;
};

CHAOS_VAR std::vector<OriginalSpread> originalValues;

static float *GetWeaponAccuracySpread(DWORD64 weapon)
{
	return reinterpret_cast<float *>(weapon + 0x74);
}

static float *GetWeaponBatchSpread(DWORD64 weapon)
{
    return reinterpret_cast<float *>(weapon + 0x124);
}

static void OnStart()
{
	const auto weapons = Memory::GetAllWeaponPointers();

	originalValues.clear();

	for (size_t i = 0; i < weapons->count; i++)
	{
		const auto weapon = weapons->elements[i];
		originalValues.emplace_back(*GetWeaponAccuracySpread(weapon), *GetWeaponBatchSpread(weapon));
		*GetWeaponAccuracySpread(weapon) = 250.0f;
		*GetWeaponBatchSpread(weapon) = 250.0f;
	}
}

static void OnStop()
{
	const auto weapons = Memory::GetAllWeaponPointers();

	for (size_t i = 0; i < weapons->count; i++)
	{
		const auto weapon       = weapons->elements[i];
        const auto info = originalValues[i];
		*GetWeaponAccuracySpread(weapon) = info.accuracySpread;
		*GetWeaponBatchSpread(weapon) = info.batchSpread;
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