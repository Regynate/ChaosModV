#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void SpawnEvilPoliceMicheal()
{
	auto const player     = PLAYER::PLAYER_PED_ID();
	auto const coords     = ENTITY::GET_ENTITY_COORDS(player, false);

	auto const michaelHash = GET_HASH_KEY("player_zero");
	auto const micheal     = CreatePoolPed(4, michaelHash, coords.x, coords.y, coords.z, 0);

	PED::SET_PED_COMPONENT_VARIATION(micheal, 0, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 1, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 2, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 3, 10, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 4, 9, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 5, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 6, 1, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 7, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 8, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 9, 6, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 10, 3, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(micheal, 11, 0, 0, 0);

	auto const assaultRifleHash = GET_HASH_KEY("weapon_assaultrifle");
	WEAPON::GIVE_DELAYED_WEAPON_TO_PED(micheal, assaultRifleHash, 200, true);

	TASK_COMBAT_PED(micheal, player, 0, 16);
	SET_PED_RELATIONSHIP_GROUP_HASH(micheal, GET_HASH_KEY("HATES_PLAYER"));
	TASK_GO_TO_ENTITY(micheal, player, -1, 2.0f, 3.0f, 0, 0);
}

static void SpawnEvilPoliceFranklin()
{
	auto const player       = PLAYER::PLAYER_PED_ID();
	auto const coords       = ENTITY::GET_ENTITY_COORDS(player, false);

	auto const franklinHash = GET_HASH_KEY("player_one");
	auto const franklin     = CreatePoolPed(4, franklinHash, coords.x, coords.y, coords.z, 0);

	PED::SET_PED_COMPONENT_VARIATION(franklin, 0, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 1, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 2, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 3, 15, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 4, 19, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 5, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 6, 13, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 7, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 8, 13, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 9, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 10, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(franklin, 11, 0, 0, 0);

	auto const assaultRifleHash = GET_HASH_KEY("weapon_pistol");
	WEAPON::GIVE_DELAYED_WEAPON_TO_PED(franklin, assaultRifleHash, 200, true);

	TASK_COMBAT_PED(franklin, player, 0, 16);
	SET_PED_RELATIONSHIP_GROUP_HASH(franklin, GET_HASH_KEY("HATES_PLAYER"));
	TASK_GO_TO_ENTITY(franklin, player, -1, 2.0f, 3.0f, 0, 0);
}

static void SpawnEvilPoliceTrevor()
{
	auto const player     = PLAYER::PLAYER_PED_ID();
	auto const coords     = ENTITY::GET_ENTITY_COORDS(player, false);

	auto const trevorHash = GET_HASH_KEY("player_two");
	auto const trevor     = CreatePoolPed(4, trevorHash, coords.x, coords.y, coords.z, 0);

	PED::SET_PED_COMPONENT_VARIATION(trevor, 0, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 1, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 2, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 3, 7, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 4, 7, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 5, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 6, 1, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 7, 0, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 8, 14, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 9, 3, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 10, 3, 0, 0);
	PED::SET_PED_COMPONENT_VARIATION(trevor, 11, 0, 0, 0);

	auto const pumpShotgunHash = GET_HASH_KEY("weapon_pumpshotgun");
	WEAPON::GIVE_DELAYED_WEAPON_TO_PED(trevor, pumpShotgunHash, 200, true);

	TASK_COMBAT_PED(trevor, player, 0, 16);
	SET_PED_RELATIONSHIP_GROUP_HASH(trevor, GET_HASH_KEY("HATES_PLAYER"));
	TASK_GO_TO_ENTITY(trevor, player, -1, 2.0f, 3.0f, 0, 0);
}

static void OnStart()
{
	SpawnEvilPoliceMicheal();
	SpawnEvilPoliceFranklin();
	SpawnEvilPoliceTrevor();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Spawn Evil Trio",
		.Id = "peds_spawn_evil_trio",
        .EffectGroupType = EffectGroupType::SpawnEnemy
	}
);