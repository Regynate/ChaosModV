#include <stdafx.h>

#define FAN_GIRLS_AMOUNT 5

static void OnStart()
{
	static const Hash modelHash = GET_HASH_KEY("a_f_y_topless_01");

	Hash relationshipGroup;
	ADD_RELATIONSHIP_GROUP("_FAN_GIRLS", &relationshipGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(0, relationshipGroup, GET_HASH_KEY("PLAYER"));
	SET_RELATIONSHIP_BETWEEN_GROUPS(0, GET_HASH_KEY("PLAYER"), relationshipGroup);

	Ped playerPed = PLAYER_PED_ID();
	// Vector3 playerPos = GET_ENTITY_COORDS(playerPed, false);

	for (int i = 0; i < FAN_GIRLS_AMOUNT; i++)
	{
		Vector3 spawnPos = GetCoordAround(playerPed, g_Random.GetRandomInt(0, 360), 1, 0, true);
		Ped ped = CreatePoolPed(28, modelHash, spawnPos.x, spawnPos.y, spawnPos.z, 0.f);
		if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
		{
			SET_PED_INTO_VEHICLE(ped, GET_VEHICLE_PED_IS_IN(playerPed, false), -2);
		}
		SET_PED_RELATIONSHIP_GROUP_HASH(ped, relationshipGroup);
		SET_PED_AS_GROUP_MEMBER(ped, GET_PLAYER_GROUP(PLAYER_ID()));
		SET_PED_HEARING_RANGE(ped, 9999.f);
	}
}

static RegisterEffect registerEffect(EFFECT_SPAWN_FAN_GIRLS, OnStart, EffectInfo
	{
		.Name = "Spawn Fan Girls",
		.Id = "peds_spawnfangirls",
		.EEffectGroupType = EEffectGroupType::SpawnCompanion
	}
);