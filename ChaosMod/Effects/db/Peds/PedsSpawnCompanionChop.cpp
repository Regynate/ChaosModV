#include <stdafx.h>

#include "Util/Peds.h"

static void OnStart()
{
	static const Hash modelHash = "a_c_chop"_hash;

	Ped playerPed     = PLAYER_PED_ID();
	Vector3 playerPos = GET_ENTITY_COORDS(playerPed, false);

	Ped ped = CreatePoolPed(28, modelHash, playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(playerPed));
	SET_PED_COMBAT_ATTRIBUTES(ped, 0, false);
	SetCompanionRelationship(ped, "COMPANION_CHOP");
	SET_PED_HEARING_RANGE(ped, 9999.f);

	SET_PED_AS_GROUP_MEMBER(ped, GET_PLAYER_GROUP(PLAYER_ID()));
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, EffectInfo
	{
		.Name = "Spawn Companion Doggo",
		.Id = "spawn_chop",
		.EffectGroupType = EEffectGroupType::SpawnCompanion
	}
);