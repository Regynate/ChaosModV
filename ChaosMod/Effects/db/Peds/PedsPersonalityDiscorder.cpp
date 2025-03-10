#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

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

	auto const max             = characters.size() - 1;
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
