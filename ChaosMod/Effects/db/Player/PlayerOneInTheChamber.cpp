#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>


CHAOS_VAR std::vector<Ped> killedPeds;
CHAOS_VAR Hash goldenGunHash = GET_HASH_KEY("weapon_doubleaction");
CHAOS_VAR int goldenGunAmmo  = 1;

CHAOS_VAR std::vector<std::pair<Hash, int>> storedWeapons;
CHAOS_VAR std::vector<Hash> WEAPON_HASHES = {
	0x92A27487, // dagger
	0x958A4A8F, // bat
	0xF9E6AA4B, // bottle
	0x84BD7BFD, // crowbar
	0xA2719263, // unarmed
	0x8BB05FD7, // flashlight
	0x440E4788, // golfclub
	0x4E875F73, // hammer
	0xF9DCBF2D, // hatchet
	0xD8DF3C3C, // knuckle
	0x99B507EA, // knife
	0xDD5DF8D9, // machete
	0xDFE37640, // switchblade
	0x678B81B1, // nightstick
	0x19044EE0, // wrench
	0xCD274149, // battleaxe
	0x94117305, // poolcue
	0x3813FC08, // stone_hatchet
	0x6589186A, // weapon_candycane
	0xDAC00025, // weapon_stunrod
	0x1B06D571, // pistol
	0xBFE256D4, // pistol_mk2
	0x5EF9FEC4, // combatpistol
	0x22D8FE39, // appistol
	0x3656C8C1, // stungun
	0x99AEEB3B, // pistol50
	0xBFD21232, // snspistol
	0x88374054, // snspistol_mk2
	0xD205520E, // heavypistol
	0x83839C4,  // vintagepistol
	0x47757124, // flaregun
	0xDC4DB296, // marksmanpistol
	0xC1B3C3D1, // revolver
	0xCB96392F, // revolver_mk2
	0x97EA20B8, // doubleaction
	0xAF3696A1, // raypistol
	0x2B5EF5EC, // ceramicpistol
	0x917F6C8C, // navyrevolver
	0x13532244, // microsmg
	0x2BE6766B, // smg
	0x78A97CD0, // smg_mk2
	0xEFE7E2DF, // assaultsmg
	0xA3D4D34,  // combatpdw
	0xDB1AA450, // machinepistol
	0xBD248B55, // minismg
	0x476BF155, // raycarbine
	0x1D073A89, // pumpshotgun
	0x555AF99A, // pumpshotgun_mk2
	0x7846A318, // sawnoffshotgun
	0xE284C527, // assaultshotgun
	0x9D61E50F, // bullpupshotgun
	0xA89CB99E, // musket
	0x3AABBBAA, // heavyshotgun
	0xEF951FBB, // dbshotgun
	0x12E82D3D, // autoshotgun
	0xBFEFFF6D, // assaultrifle
	0x394F415C, // assaultrifle_mk2
	0x83BF0278, // carbinerifle
	0xFAD1F1C9, // carbinerifle_mk2
	0xAF113F99, // advancedrifle
	0xC0A3098D, // specialcarbine
	0x969C3D67, // specialcarbine_mk2
	0x7F229F94, // bullpuprifle
	0x84D6FAFD, // bullpuprifle_mk2
	0x624FE830, // compactrifle
	0x9D07F764, // mg
	0x7FD62962, // combatmg
	0xDBBD7280, // combatmg_mk2
	0x61012683, // gusenberg
	0x5FC3C11,  // sniperrifle
	0xC472FE2,  // heavysniper
	0xA914799,  // heavysniper_mk2
	0xC734385A, // marksmanrifle
	0x6A6C02E0, // marksmanrifle_mk2
	0xB1CA77B1, // rpg
	0xA284510B, // grenadelauncher
	0x4DD2DC56, // grenadelauncher_smoke
	0x42BF8A85, // minigun
	0x7F7497E5, // firework
	0x6D544C99, // railgun
	0x63AB0442, // hominglauncher
	0x781FE4A,  // compactlauncher
	0xB62D1F67, // rayminigun
	0x93E220BD, // grenade
	0xA0973D5E, // bzgas
	0xFDBC8A50, // smokegrenade
	0x497FACC3, // flare
	0x24B17070, // molotov
	0x2C3731D9, // stickybomb
	0xAB564B93, // proxmine
	0x787F0BB,  // snowball
	0xBA45E8B8, // pipebomb
	0x23C9F95C, // ball
	0x34A67B97, // petrolcan
	0x60EC506,  // fireextinguisher
	0xFBAB5776, // parachute
	0xBA536372  // hazardcan
};

static void StoreAndRemoveWeapons()
{
	auto const player = PLAYER_PED_ID();
	storedWeapons.clear();

	for (auto const weaponHash : WEAPON_HASHES)
	{
		if (HAS_PED_GOT_WEAPON(player, weaponHash, false))
		{
			auto const ammo = GET_AMMO_IN_PED_WEAPON(player, weaponHash);
			storedWeapons.push_back({ weaponHash, ammo });
		}
	}

	REMOVE_ALL_PED_WEAPONS(player, true);
}

static void GiveGoldenGun()
{
	auto const player = PLAYER_PED_ID();
	GIVE_WEAPON_TO_PED(player, goldenGunHash, goldenGunAmmo, false, true);
}

static void RestoreWeapons()
{
	auto const player = PLAYER_PED_ID();

	REMOVE_WEAPON_FROM_PED(player, goldenGunHash);

	for (auto const &weaponData : storedWeapons)
		GIVE_WEAPON_TO_PED(player, weaponData.first, weaponData.second, false, true);
}

static void OnStart()
{
	StoreAndRemoveWeapons();
	GiveGoldenGun();
}

static void OnStop()
{
	RestoreWeapons();
}

static void GiveAmmoOnPedKilled()
{
	for (auto const ped : GetAllPeds())
	{
		auto const player = PLAYER_PED_ID();

		if (IS_PED_DEAD_OR_DYING(ped, false))
		{
			auto const pedSourceOfDeath = GET_PED_SOURCE_OF_DEATH(ped);
			if (pedSourceOfDeath == player)
			{
				auto it = std::find(killedPeds.begin(), killedPeds.end(), ped);
				if (it == killedPeds.end())
				{
					killedPeds.push_back(ped);

					goldenGunAmmo;
					SET_AMMO_IN_CLIP(player, goldenGunHash, goldenGunAmmo);
				}
			}
		}
	}
}

static void OnTick()
{
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