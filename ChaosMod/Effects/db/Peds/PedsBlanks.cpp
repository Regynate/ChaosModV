/*
    Effect By OnlyRealNubs & MoneyWasted
*/

#include "stdafx.h"

#include "Effects/Register/RegisterEffect.h"

#include "Memory/Weapon.h"

CHAOS_VAR const float fRange = 0.1f;

struct BlanksWeapon
{
	Hash weapon;
	float ogRange;
	float ogDamage = -1;
};

std::vector<BlanksWeapon> weapons;

static bool WepMapped(Hash &wep)
{
	auto iter = std::find_if(weapons.begin(), weapons.end(), [&](const BlanksWeapon &tw) { return tw.weapon == wep; });
	return iter != weapons.end();
}

static Hash GetPedWepHash(Ped ped)
{
	Hash h;
	GET_CURRENT_PED_WEAPON(ped, &h, true);
	return h;
}

static void OnStop()
{
	for (BlanksWeapon &wep : weapons)
	{
		Memory::SetWeaponRange(wep.weapon, wep.ogRange);
		if (wep.ogDamage > -1)
			Memory::SetWeaponDamage(wep.weapon, wep.ogDamage);
	}
	weapons.clear();
}

static void OnTick()
{
	for (Ped ped : GetAllPeds())
	{
		Hash wepHash = GetPedWepHash(ped);
		if (!WepMapped(wepHash))
		{
			if (!IS_PED_ARMED(ped, 7))
				continue;
			else if (!Memory::IsWeaponRangable(wepHash))
			{
				Memory::SetWeaponDamage(wepHash, 0.f);
				weapons.push_back(
				    BlanksWeapon(wepHash, Memory::GetWeaponRange(wepHash), Memory::GetWeaponDamage(wepHash)));
				continue;
			}
			weapons.push_back(BlanksWeapon(wepHash, Memory::GetWeaponRange(wepHash)));
			Memory::SetWeaponRange(wepHash, fRange);
		}
	}
}

// clang-format off

REGISTER_EFFECT(nullptr, OnStop, OnTick,
    {
        .Name            = "Blanks",
        .Id              = "peds_blanks",
        .IsTimed         = true,
        .IsShortDuration = true,
    }
);