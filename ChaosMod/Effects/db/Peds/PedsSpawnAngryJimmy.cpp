/*
    Effect by MoneyWasted
*/

#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	static constexpr Hash modelHash = 1459905209;

	Ped playerPed                   = PLAYER_PED_ID();
	Vector3 playerPos               = GET_ENTITY_COORDS(playerPed, false);

	static const Hash playerGroup   = "PLAYER"_hash;
	static const Hash civGroup      = "CIVMALE"_hash;
	static const Hash femCivGroup   = "CIVFEMALE"_hash;

	Hash relationshipGroup;
	ADD_RELATIONSHIP_GROUP("_HOSTILE_JIMMY", &relationshipGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(5, relationshipGroup, playerGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(5, relationshipGroup, civGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(5, relationshipGroup, femCivGroup);

	Ped ped = CreatePoolPed(4, modelHash, playerPos.x, playerPos.y, playerPos.z, 0.f);
	CurrentEffect::SetEffectSoundPlayOptions({ .PlayType = EffectSoundPlayType::FollowEntity, .Entity = ped });
	if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
		SET_PED_INTO_VEHICLE(ped, GET_VEHICLE_PED_IS_IN(playerPed, false), -2);

	SET_PED_RELATIONSHIP_GROUP_HASH(ped, relationshipGroup);
	SET_PED_HEARING_RANGE(ped, 9999.f);
	SET_PED_CONFIG_FLAG(ped, 281, true);

	SET_ENTITY_PROOFS(ped, false, true, true, false, false, false, false, false);

	SET_PED_COMBAT_ATTRIBUTES(ped, 5, true);
	SET_PED_COMBAT_ATTRIBUTES(ped, 46, true);

	SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(ped, false);
	SET_RAGDOLL_BLOCKING_FLAGS(ped, 5);
	SET_PED_SUFFERS_CRITICAL_HITS(ped, false);

	GIVE_WEAPON_TO_PED(ped, "WEAPON_COMBATMG"_hash, 9999, true, true);
	TASK_COMBAT_PED(ped, playerPed, 0, 16);

	SET_PED_FIRING_PATTERN(ped, 0xC6EE6B4C);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Spawn Jealous Jimmy",
		.Id = "peds_angryjimmy",
		.EffectGroupType = EffectGroupType::SpawnEnemy
	}
);