#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static std::vector<std::int32_t> originalClothes;

static void SaveOriginalClothes(std::int32_t player)
{
	originalClothes.clear();
	for (int i = 0; i <= 11; i++)
		originalClothes.push_back(GET_PED_DRAWABLE_VARIATION(player, i));
}

static void RestoreOriginalClothes(std::int32_t player)
{
	if (originalClothes.empty())
		return;
	for (int i = 0; i <= 11; i++)
		SET_PED_COMPONENT_VARIATION(player, i, originalClothes[i], 0, 0);
}

static void RandomizePlayerClothes(std::int32_t player)
{
	for (int i = 0; i <= 11; i++)
	{
		auto const maxDrawable = GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(player, i);
		if (maxDrawable > 0)
		{
			auto const randomVariation = GET_RANDOM_INT_IN_RANGE(0, maxDrawable);
			SET_PED_COMPONENT_VARIATION(player, i, randomVariation, 0, 0);
		}
	}
}

static void OnStart()
{
	auto const player = PLAYER_PED_ID();
	SaveOriginalClothes(player);
}

static void OnStop()
{
	auto const player = PLAYER_PED_ID();
	RestoreOriginalClothes(player);
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	RandomizePlayerClothes(player);
	WAIT(5000);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Randomize Clothes Every 5 Seconds", 
        .Id = "player_randomize_player_clothes", 
        .IsTimed = true
    }
);
