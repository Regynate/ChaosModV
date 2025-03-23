#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>
static void InvinciblePedsWithNoRagdoll()
{
	for (auto const ped : GetAllPeds())
	{
		auto const player = PLAYER_PED_ID();
		if (ped == player)
			return;
		SET_ENTITY_INVINCIBLE(ped, true);
		SET_PED_CAN_RAGDOLL(ped, false);
	}
}

static void ResetInvinciblePedsWithNoRagdoll()
{
	for (auto const ped : GetAllPeds())
	{
		auto const player = PLAYER_PED_ID();
		if (ped == player)
			return;
		SET_ENTITY_INVINCIBLE(ped, false);
		SET_PED_CAN_RAGDOLL(ped, true);
	}
}

static void OnStart()
{
}

static void OnStop()
{
	ResetInvinciblePedsWithNoRagdoll();
}

static void OnTick()
{
	auto const player          = PLAYER_PED_ID();
	auto const playerId        = PLAYER_ID();

	auto constexpr INPUT_ENTER = 23;
	auto constexpr INPUT_AIM   = 25;

	SET_CURRENT_PED_WEAPON(player, GET_HASH_KEY("WEAPON_UNARMED"), true);
	HIDE_HUD_COMPONENT_THIS_FRAME(19);

	DISABLE_CONTROL_ACTION(0, INPUT_ENTER, true);

	CLEAR_PLAYER_WANTED_LEVEL(playerId);

	SKIP_TO_NEXT_SCRIPTED_CONVERSATION_LINE();

	InvinciblePedsWithNoRagdoll();
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
    {
        .Name = "Kid Friendly", 
        .Id = "kid_friendly", 
        .IsTimed = true
    }
);
