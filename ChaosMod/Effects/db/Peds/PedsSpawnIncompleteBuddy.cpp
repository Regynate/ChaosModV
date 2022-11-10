/*
	Effect by Reguas
*/

#include <stdafx.h>

static void OnStart()
{
	Hash relationshipGroup;
	ADD_RELATIONSHIP_GROUP("_COMPANION_INCOMPLETE_BUDDY", &relationshipGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(0, relationshipGroup, GET_HASH_KEY("PLAYER"));
	SET_RELATIONSHIP_BETWEEN_GROUPS(0, GET_HASH_KEY("PLAYER"), relationshipGroup);

	Ped playerPed     = PLAYER_PED_ID();
	Vector3 playerPos = GET_ENTITY_COORDS(playerPed, false);

	Hash model        = GET_HASH_KEY(g_Random.GetRandomInt(0, 1) == 0 ? "mp_m_marston_01" : "mp_m_niko_01");

	Ped ped = CreatePoolPed(4, model, playerPos.x, playerPos.y, playerPos.z, GET_ENTITY_HEADING(playerPed));
	if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
	{
		SET_PED_INTO_VEHICLE(ped, GET_VEHICLE_PED_IS_IN(playerPed, false), -2);
	}

	SET_PED_SUFFERS_CRITICAL_HITS(ped, false);

	SET_PED_RELATIONSHIP_GROUP_HASH(ped, relationshipGroup);
	SET_PED_HEARING_RANGE(ped, 10.f);

	SET_PED_AS_GROUP_MEMBER(ped, GET_PLAYER_GROUP(PLAYER_ID()));

	SET_PED_ACCURACY(ped, 2);
	SET_PED_FIRING_PATTERN(ped, 0xC6EE6B4C);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, EffectInfo
	{
		.Name = "Spawn Incomplete Buddy",
		.Id = "peds_spawnincbuddy",
		.EffectGroupType = EEffectGroupType::SpawnCompanion
	}
);