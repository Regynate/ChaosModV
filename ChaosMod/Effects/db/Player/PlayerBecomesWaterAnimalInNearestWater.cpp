#include "Components/EffectDispatcher.h"
#include "Effects/Register/RegisterEffect.h"
#include <ranges>
#include <stdafx.h>

static Vector3 GetClosestQuad()
{
	struct Quad
	{
		std::int16_t minx;
		std::int16_t miny;
		std::int16_t maxx;
		std::int16_t maxy;
		std::uint32_t alpha;
		char _0x000C[8];
		float height;
		char _0x0024[4];
	};
#undef max
	auto constexpr ocean = static_cast<std::uintptr_t>(0x227e640);

	auto const base      = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
	auto const pool      = *reinterpret_cast<std::uint64_t *>(base + ocean);
	auto const size      = *reinterpret_cast<std::uint16_t *>(base + ocean + static_cast<std::uintptr_t>(0x8));

	auto const myCoords  = GET_ENTITY_COORDS(PLAYER_PED_ID(), true);
	auto closestDistance = std::numeric_limits<float>::max();
	Quad *closestQuad {};

	constexpr std::array<std::uint16_t, 212> ignoredIndices = {
		293, 292, 279, 289, 492, 547, 221, 219, 220, 223, 172, 176, 181, 182, 176, 179, 178, 119, 118, 127,
		125, 124, 123, 80,  79,  83,  85,  38,  42,  33,  46,  45,  50,  52,  51,  56,  86,  87,  88,  89,
		91,  93,  95,  101, 99,  102, 146, 145, 149, 151, 152, 156, 199, 200, 198, 209, 203, 234, 225, 236,
		306, 303, 295, 316, 317, 310, 387, 386, 395, 392, 455, 494, 512, 514, 517, 516, 563, 557, 555, 608,
		607, 650, 706, 604, 648, 716, 711, 712, 706, 708, 707, 702, 784, 783, 782, 781, 779, 778, 777, 776,
		773, 770, 759, 756, 755, 766, 763, 747, 749, 752, 751, 741, 742, 692, 693, 694, 698, 695, 696, 679,
		680, 681, 691, 688, 685, 672, 671, 645, 644, 642, 639, 638, 637, 636, 635, 633, 634, 632, 630, 631,
		629, 628, 629, 577, 575, 576, 573, 572, 625, 624, 571, 574, 570, 524, 523, 503, 504, 478, 481, 484,
		482, 419, 421, 431, 430, 346, 345, 344, 343, 342, 264, 263, 270, 272, 268, 265, 222, 218, 169, 167,
		168, 164, 216, 217, 250, 485, 426, 430, 346, 344, 529, 530, 528, 527, 526, 525, 578, 579, 580, 536,
		432, 347, 339, 420, 273, 271, 534, 358, 581, 457, 646, 553
	};

	for (auto const i : std::views::iota(std::uint16_t(0), size))
	{
		if (std::ranges::find(ignoredIndices, i) != ignoredIndices.end())
			continue;

		auto constexpr offset  = static_cast<std::uint16_t>(0x1C);
		auto const currentQuad = reinterpret_cast<Quad *>(pool + (i * offset));
		if (!currentQuad)
			continue;

		auto const quadCoords =
		    Vector3 { static_cast<float>(currentQuad->minx + currentQuad->maxx) / 2.0f,
			          static_cast<float>(currentQuad->miny + currentQuad->maxy) / 2.0f, currentQuad->height };

		auto const xDiff          = quadCoords.x - myCoords.x;
		auto const yDiff          = quadCoords.y - myCoords.y;
		auto const zDiff          = quadCoords.z - myCoords.z;
		auto const distanceToQuad = std::sqrt(xDiff * xDiff + yDiff * yDiff + zDiff * zDiff);

		if (distanceToQuad < closestDistance)
		{
			closestDistance = distanceToQuad;
			closestQuad     = currentQuad;
		}
	}

	if (!closestQuad)
		return {};

	auto const quadCoords =
	    Vector3 { static_cast<float>(closestQuad->minx + closestQuad->maxx) / 2.0f,
		          static_cast<float>(closestQuad->miny + closestQuad->maxy) / 2.0f, closestQuad->height };

	return Vector3(quadCoords.x, quadCoords.y, quadCoords.z);
}

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

	SET_PLAYER_MODEL(PLAYER_ID(), hash);
}

CHAOS_VAR Hash previousModel {};
CHAOS_VAR bool hasDiedOrFinished { false };

static void SpawnSharkOnChance()
{
	auto const player = PLAYER_PED_ID();
	auto const result = GET_RANDOM_INT_IN_RANGE(0, 100);
	auto const coords     = GET_ENTITY_COORDS(player, false);
	auto const sharkModel = GET_HASH_KEY("a_c_sharktiger");
	if (result == 69)
		CreatePoolPed(28, sharkModel, coords.x, coords.y, coords.z, 0.f);
	
}

static void OnStart()
{
	auto const player        = PLAYER_PED_ID();

	auto const oldModel      = GET_ENTITY_MODEL(player);
	auto const michealModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	if (oldModel != michealModel && oldModel != franklinModel && oldModel != trevorModel)
		previousModel = franklinModel;
	else
		previousModel = oldModel;

	static std::array<Hash, 6> waterAnimals { GET_HASH_KEY("a_c_dolphin"),     GET_HASH_KEY("a_c_fish"),
		                                      GET_HASH_KEY("a_c_sharkhammer"), GET_HASH_KEY("a_c_humpback"),
		                                      GET_HASH_KEY("a_c_killerwhale"), GET_HASH_KEY("a_c_stingray") };

	auto constexpr max           = waterAnimals.size();
	auto const randomWaterAnimal = GET_RANDOM_INT_IN_RANGE(0, max);
	auto const selectedAnimal    = waterAnimals[randomWaterAnimal];

	LoadModel(selectedAnimal);

	auto const swimming         = IS_PED_SWIMMING(player);
	if (swimming)
	{
		auto const coords = GET_ENTITY_COORDS(player, false);
		SET_ENTITY_COORDS(player, coords.x, coords.y, coords.z -3.f, false, false, false, false);
		StoreWeapons();
		SetPlayerModel(selectedAnimal);
		SpawnSharkOnChance();
		return;
	}

	auto const nearestWaterQuad = GetClosestQuad();

	for (auto const _ : std::ranges::iota_view { 0, 5 })
	{
		SET_ENTITY_COORDS(player, nearestWaterQuad.x, nearestWaterQuad.y, nearestWaterQuad.z - 5.0f, false, false,
		                  false, false);
		WAIT(0);
	}

	while (!IS_ENTITY_IN_WATER(player))
		WAIT(500);

	while (!IS_PED_SWIMMING(player))
		WAIT(500);
	StoreWeapons();
	SetPlayerModel(selectedAnimal);

	SpawnSharkOnChance();
}

static void OnStop()
{
	if (previousModel == 0)
		return;
	SetPlayerModel(previousModel);
	RestoreWeapons();
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();

	if (hasDiedOrFinished)
		return;

	if (IS_PED_DEAD_OR_DYING(player, false))
	{

		SetPlayerModel(previousModel);
		RestoreWeapons();

		hasDiedOrFinished = true;

		WAIT(10000);
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Player Becomes Water Animal In Nearest Water", 
        .Id = "player_becomes_water_animal", 
        .IsTimed = true,
		.IsShortDuration = true
    }
);
