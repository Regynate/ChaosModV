#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static std::vector<std::pair<Hash, int>> storedWeapons;
static std::vector<Hash> WEAPON_HASHES = {
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

static void StoreWeapons()
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
}

static void RestoreWeapons()
{
	auto const player = PLAYER_PED_ID();

	for (auto const &weaponData : storedWeapons)
		GIVE_WEAPON_TO_PED(player, weaponData.first, weaponData.second, false, true);
}

static void SetPlayerModel(const std::uint32_t hash)
{
	if (!IS_MODEL_IN_CDIMAGE(hash))
		return;
	if (!IS_MODEL_VALID(hash))
		return;
	while (!HAS_MODEL_LOADED(hash))
	{
		REQUEST_MODEL(hash);
		WAIT(0);
	}

	StoreWeapons();
	SET_PLAYER_MODEL(PLAYER_ID(), hash);
	RestoreWeapons();
}

static std::array<std::uint32_t, 3> characters { GET_HASH_KEY("player_zero"), GET_HASH_KEY("player_one"),
	                                             GET_HASH_KEY("player_two") };

static std::uint32_t previousModel {};

static void OnStart()
{
	auto const player        = PLAYER_PED_ID();
	auto const playerModel   = GET_ENTITY_MODEL(player);

	auto const michealModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	if (playerModel != michealModel && playerModel != franklinModel && playerModel != trevorModel)
		previousModel = franklinModel;

	previousModel = playerModel;
}

static void OnStop()
{
	SetPlayerModel(previousModel);
}

static void OnTick()
{
	std::int32_t vehicle {};
	auto player            = PLAYER_PED_ID();

	auto const isInVehicle = IS_PED_IN_ANY_VEHICLE(player, true);
	if (isInVehicle)
		vehicle = GET_VEHICLE_PED_IS_IN(player, false);

	auto const max             = characters.size();
	auto const randomCharacter = GET_RANDOM_INT_IN_RANGE(0, max);

	SetPlayerModel(characters[randomCharacter]);

	auto const randomWaitTime = GET_RANDOM_INT_IN_RANGE(5000, 10000);
	WAIT(randomWaitTime);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Personality Disorder", 
        .Id = "peds_personality_disorder", 
        .IsTimed = true
    }
);
