#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"


void SetHitmanWeapons()
{
	auto const player = PLAYER_PED_ID();

	REMOVE_ALL_PED_WEAPONS(player, true);

	auto const pistol     = GET_HASH_KEY("weapon_pistol");
	auto const suppressor = GET_HASH_KEY("COMPONENT_AT_PI_SUPP_02");
	GIVE_DELAYED_WEAPON_TO_PED(player, pistol, 2000, true);
	GIVE_WEAPON_COMPONENT_TO_PED(player, pistol, suppressor);

	auto const stickyBomb = GET_HASH_KEY("weapon_stickybomb");
	GIVE_DELAYED_WEAPON_TO_PED(player, stickyBomb, 1, false);

	auto const grenade = GET_HASH_KEY("weapon_grenade");
	GIVE_DELAYED_WEAPON_TO_PED(player, grenade, 1, false);

	auto const knuckleDusters = GET_HASH_KEY("weapon_knuckle");
	GIVE_DELAYED_WEAPON_TO_PED(player, knuckleDusters, 1, false);

	auto const knife = GET_HASH_KEY("weapon_knife");
	GIVE_DELAYED_WEAPON_TO_PED(player, knife, 1, false);
}

void SpawnMaxedZType()
{
	auto const player     = PLAYER_PED_ID();
	auto const coords     = GET_ENTITY_COORDS(player, false);
	auto const heading    = GET_ENTITY_HEADING(player);

	auto const ZTypeModel = GET_HASH_KEY("ztype");
	LoadModel(ZTypeModel);
	auto const ZType = CreatePoolVehicle(ZTypeModel, coords.x + 2, coords.y + 2, coords.z, heading);

	SET_VEHICLE_NUMBER_PLATE_TEXT(ZType, "AgentMoo");

	for (int i = 0; i < 14; i++)
	{
		SET_VEHICLE_MOD_KIT(ZType, 0);
		int numMods = GET_NUM_VEHICLE_MODS(ZType, i);
		if (numMods > 0)
			SET_VEHICLE_MOD(ZType, i, numMods - 1, false);
	}

	SET_VEHICLE_MOD_KIT(ZType, 0);
	SET_VEHICLE_MOD(ZType, 16, GET_NUM_VEHICLE_MODS(ZType, 16) - 1, false);

	TOGGLE_VEHICLE_MOD(ZType, 18, true);
	SET_VEHICLE_TYRES_CAN_BURST(ZType, false);
	SET_VEHICLE_WINDOW_TINT(ZType, 1);
	SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(ZType, 0, 0, 0);
	SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(ZType, 0, 0, 0);
}

void SetFranklinHitman()
{
	auto const player = PLAYER_PED_ID();
	// Hair & Beard
	SET_PED_COMPONENT_VARIATION(player, 1, 0, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, 2, 5, 0, 0);

	// Suit
	SET_PED_COMPONENT_VARIATION(player, 3, 23, 1, 0);
	SET_PED_COMPONENT_VARIATION(player, 4, 15, 1, 0);
	SET_PED_COMPONENT_VARIATION(player, 6, 8, 0, 0);

	SetHitmanWeapons();
	SpawnMaxedZType();
}

void SetMichealHitman()
{
	auto const player = PLAYER_PED_ID();
	// Hair & Beard
	SET_PED_COMPONENT_VARIATION(player, 1, 0, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, 2, 1, 0, 0);

	// Suit
	SET_PED_COMPONENT_VARIATION(player, 3, 0, 9, 0);
	SET_PED_COMPONENT_VARIATION(player, 4, 0, 9, 0);

	SetHitmanWeapons();
	SpawnMaxedZType();
}

void SetTrevorHitman()
{
	auto const player = PLAYER_PED_ID();
	// Hair & Beard
	SET_PED_COMPONENT_VARIATION(player, 1, 0, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, 2, 1, 0, 0);

	// Suit
	SET_PED_COMPONENT_VARIATION(player, 3, 27, 1, 0);
	SET_PED_COMPONENT_VARIATION(player, 4, 20, 1, 0);
	SET_PED_COMPONENT_VARIATION(player, 6, 9, 0, 0);
	SET_PED_COMPONENT_VARIATION(player, 8, 14, 0, 0);

	SetHitmanWeapons();
	SpawnMaxedZType();
}

void SetPlayerHitman()
{
	auto const player        = PLAYER_PED_ID();
	auto const playerModel   = GET_ENTITY_MODEL(player);

	auto const michealModel  = GET_HASH_KEY("player_zero");
	auto const franklinModel = GET_HASH_KEY("player_one");
	auto const trevorModel   = GET_HASH_KEY("player_two");

	switch (playerModel)
	{
	case michealModel:
		SetMichealHitman();
		break;
	case franklinModel:
		SetFranklinHitman();
		break;
	case trevorModel:
		SetTrevorHitman();
	}
}

static void OnStart() {
	SetPlayerHitman();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "What If I Was The Hitman All Along?",
		.Id = "player_what_if_i_was_the_hitman_all_along"
	}
);