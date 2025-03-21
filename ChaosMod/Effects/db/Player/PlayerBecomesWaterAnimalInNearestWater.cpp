#include "Effects/Register/RegisterEffect.h"
#include "Components/EffectDispatcher.h"
#include <stdafx.h>
#include <ranges>

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

	for (auto const i : std::views::iota(std::uint16_t(0), size))
	{
		if (i == 293 || i == 292 || i == 279 || i == 492 || i == 547)
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
			LOG(std::format("{}", i));
		}
	}
	
	if (!closestQuad)
		return {};

	auto const quadCoords =
	    Vector3 { static_cast<float>(closestQuad->minx + closestQuad->maxx) / 2.0f,
		          static_cast<float>(closestQuad->miny + closestQuad->maxy) / 2.0f, closestQuad->height };

	return Vector3(quadCoords.x, quadCoords.y, quadCoords.z);
}


static std::array<std::uint32_t, 7> waterAnimals { GET_HASH_KEY("a_c_dolphin"),     GET_HASH_KEY("a_c_fish"),
	                                               GET_HASH_KEY("a_c_sharkhammer"), GET_HASH_KEY("a_c_humpback"),
	                                               GET_HASH_KEY("a_c_killerwhale"), GET_HASH_KEY("a_c_stingray"),
	                                               GET_HASH_KEY("a_c_sharktiger") };

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

static std::uint32_t previousModel {};
static bool hasDiedOrFinished { false };

static void OnStart()
{
	auto const oldModel      = GET_ENTITY_MODEL(PLAYER_PED_ID());
	auto const michealModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	if (oldModel != michealModel && oldModel != franklinModel && oldModel != trevorModel)
		previousModel = franklinModel;
	else
		previousModel = oldModel;

	auto const player            = PLAYER_PED_ID();

	auto const nearestWaterQuad  = GetClosestQuad();

	for (auto const _ : std::ranges::iota_view { 0, 5 })
	{
		SET_ENTITY_COORDS(player, nearestWaterQuad.x, nearestWaterQuad.y, nearestWaterQuad.z - 5.0f, false, false, false, false);
		WAIT(0);
	}


	auto constexpr max           = waterAnimals.size();
	auto const randomWaterAnimal = GET_RANDOM_INT_IN_RANGE(0, max);
	auto const selectedAnimal    = waterAnimals[randomWaterAnimal];
	
	while (!IS_ENTITY_IN_WATER(player))
		WAIT(500);
	
	while (!IS_PED_SWIMMING(player))
		WAIT(500);

	SetPlayerModel(selectedAnimal);
}

static void OnStop()
{
	if (previousModel == 0)
		return;
	SetPlayerModel(previousModel);
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();

	if (hasDiedOrFinished)
		return;

	if (IS_PED_DEAD_OR_DYING(player, false))
	{
		
		SetPlayerModel(previousModel);
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
