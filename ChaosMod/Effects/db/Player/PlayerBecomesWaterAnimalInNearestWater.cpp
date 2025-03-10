#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

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

	auto constexpr max           = waterAnimals.size() - 1;
	auto const randomWaterAnimal = GET_RANDOM_INT_IN_RANGE(0, max);
	auto const selectedAnimal    = waterAnimals[randomWaterAnimal];
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
        .Name = "Player Becomes Water Animal", 
        .Id = "player_becomes_water_animal", 
        .IsTimed = true
    }
);
