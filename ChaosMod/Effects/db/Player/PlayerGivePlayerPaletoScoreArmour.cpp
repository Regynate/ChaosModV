#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void GiveFranklinPaletoScoreArmour()
{
	auto const player             = PLAYER_PED_ID();
	auto constexpr torsoComponent = 3;
	auto constexpr torsoTextureId = 3;
	auto constexpr legComponent   = 4;
	auto constexpr legTextureId   = 3;
	auto constexpr gloveComponent = 5;
	auto constexpr gloveTextureId = 4;
	SET_PED_COMPONENT_VARIATION(player, torsoComponent, torsoTextureId, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, legComponent, legTextureId, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, gloveComponent, gloveTextureId, 0, 0);
}

static void GiveMichealPaletoScoreArmour()
{
	auto const player                   = PLAYER_PED_ID();
	auto constexpr torsoComponent       = 3;
	auto constexpr torsoTextureId       = 5;
	auto constexpr legComponent         = 4;
	auto constexpr legTextureId         = 5;
	auto constexpr gloveComponent       = 5;
	auto constexpr gloveTextureId       = 1;
	auto constexpr outerArmourComponent = 8;
	auto constexpr outerArmourTextureId = 5;

	SET_PED_COMPONENT_VARIATION(player, torsoComponent, torsoTextureId, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, legComponent, legTextureId, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, gloveComponent, gloveTextureId, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, outerArmourComponent, outerArmourTextureId, 0, 0);
}

static void GiveTrevorPaletoScoreArmour()
{
	auto const player                   = PLAYER_PED_ID();
	auto constexpr torsoComponent       = 3;
	auto constexpr torsoTextureId       = 2;
	auto constexpr legComponent         = 4;
	auto constexpr legTextureId         = 2;
	auto constexpr gloveComponent       = 5;
	auto constexpr gloveTextureId       = 1;
	auto constexpr outerArmourComponent = 8;
	auto constexpr outerArmourTextureId = 2;

	SET_PED_COMPONENT_VARIATION(player, torsoComponent, torsoTextureId, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, legComponent, legTextureId, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, gloveComponent, gloveTextureId, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, outerArmourComponent, outerArmourTextureId, 0, 0);
}

static void GivePlayerPaletoScoreArmour()
{
	auto const player        = PLAYER_PED_ID();
	auto const playerModel   = GET_ENTITY_MODEL(player);

	auto const michealModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	switch (playerModel)
	{
	case michealModel:
		GiveMichealPaletoScoreArmour();
		break;
	case franklinModel:
		GiveFranklinPaletoScoreArmour();
		break;
	case trevorModel:
		GiveTrevorPaletoScoreArmour();
	}
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	SET_ENTITY_HEALTH(player, 200, 0);
}

static void OnStart()
{
	GivePlayerPaletoScoreArmour();
}

static void OnStop()
{
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, OnTick, 
    {
        .Name = "Give Paleto Score Armour", 
        .Id = "player_give_paleto_score_armour", 
        .IsTimed = true
    }
);
