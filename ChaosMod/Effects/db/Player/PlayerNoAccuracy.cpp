/*
    Effect By OnlyRealNubs
*/

#include <stdafx.h>

#include <Memory/Weapon.h>

static std::vector<Hash> modifiedWeapons = {};
static float fAccuracyMult                               = 0;
static Hash playerLastWepHash                          = 0;

static Hash GetPedWeapon(Ped ped)
{
	// TODO: Get ped weapon from CWeaponInfo
	Hash wepHash;
	GET_CURRENT_PED_WEAPON(ped, &wepHash, true);
	return wepHash;
}

static bool IsWeaponNotMelee(Hash wep)
{
	uintptr_t pedWeaponInfoAddr = Memory::GetWeaponInfo(wep);
	return *reinterpret_cast<int32_t *>(pedWeaponInfoAddr + 0x58) != 4;
}

static void OnStart()
{
	fAccuracyMult = g_Random.GetRandomFloat(30.f, 80.f);
}

static void OnStop()
{
	for (auto const &wep : modifiedWeapons)
	{
		float accuracySpread = Memory::GetWeaponAccuracySpread(wep);
		float accuracyRecoil = Memory::GetWeaponAccuracy(wep);
		Memory::SetWeaponAccuracySpread(wep, accuracySpread / fAccuracyMult);
		Memory::SetWeaponAccuracy(wep, accuracyRecoil * fAccuracyMult);
	}

	modifiedWeapons.clear();
	playerLastWepHash = 0;
}

static void OnTick()
{
	static Ped playerPed = PLAYER_PED_ID();
	Hash plrCurWep       = GetPedWeapon(playerPed);

	if (plrCurWep != playerLastWepHash)
	{
		playerLastWepHash = plrCurWep;

		if (IsWeaponNotMelee(plrCurWep) && !std::count(modifiedWeapons.begin(), modifiedWeapons.end(), plrCurWep))
		{
			float wepAccuracySpread = Memory::GetWeaponAccuracySpread(plrCurWep);
			float wepAccuracyRecoil = Memory::GetWeaponAccuracy(plrCurWep);
			Memory::SetWeaponAccuracySpread(plrCurWep, wepAccuracySpread * fAccuracyMult);
			Memory::SetWeaponAccuracy(plrCurWep, wepAccuracySpread / fAccuracyMult);
			modifiedWeapons.push_back(plrCurWep);
		}
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, EffectInfo
	{
		.Name = "Bad Aim",
		.Id = "player_noaccuracy",
		.IsTimed = true
	}
);