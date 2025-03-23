#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void OnTick()
{
	auto const player   = PLAYER_PED_ID();
	auto const playerId = PLAYER_ID();

	if (!IS_PED_SHOOTING(player))
		return;

	Entity entity{};
	GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(playerId, &entity);

	if (!DOES_ENTITY_EXIST(entity))
		return;

	if (!IS_ENTITY_A_PED(entity))
		return;

	auto const group = GET_PED_GROUP_INDEX(player);
	SET_PED_AS_GROUP_MEMBER(entity, group);
	SET_PED_COMBAT_RANGE(entity, 2);
	SET_PED_ALERTNESS(entity, 100);
	SET_PED_ACCURACY(entity, 100);
	SET_PED_CAN_SWITCH_WEAPON(entity, 1);
	SET_PED_SHOOT_RATE(entity, 200);
	SET_PED_KEEP_TASK(entity, true);
	TASK_COMBAT_HATED_TARGETS_AROUND_PED(entity, 5000.f, 0);
	SET_PED_KEEP_TASK(entity, true);
	SET_PED_GENERATES_DEAD_BODY_EVENTS(entity, true);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Shot Enemies Become Friendly", 
        .Id = "peds_shot_enemies_friendly", 
        .IsTimed = true
    }
);
