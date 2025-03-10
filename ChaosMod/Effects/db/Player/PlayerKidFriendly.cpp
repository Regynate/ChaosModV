#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void ped(void (*processor)(std::int32_t))
{
	static constexpr std::int32_t MAX_ENTITIES = 100;
	std::int32_t peds[MAX_ENTITIES];
	auto const ped_count = worldGetAllPeds(peds, MAX_ENTITIES);

	for (std::int32_t const i : std::ranges::iota_view { 0, ped_count })
	{
		auto const ped_handle        = peds[i];

		auto const does_entity_exist = DOES_ENTITY_EXIST(ped_handle);
		if (!does_entity_exist)
			continue;

		processor(ped_handle);
	}
}

static void InvinciblePedsWithNoRagdoll(std::int32_t ped)
{
	auto const player = PLAYER_PED_ID();
	if (ped == player)
		return;
	SET_ENTITY_INVINCIBLE(ped, true);
	SET_PED_CAN_RAGDOLL(ped, false);
}

static void ResetInvinciblePedsWithNoRagdoll(std::int32_t ped)
{
	auto const player = PLAYER_PED_ID();
	if (ped == player)
		return;
	SET_ENTITY_INVINCIBLE(ped, false);
	SET_PED_CAN_RAGDOLL(ped, true);
}

static void OnStart()
{
}

static void OnStop()
{
	ped(ResetInvinciblePedsWithNoRagdoll);
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

	ped(InvinciblePedsWithNoRagdoll);
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
    {
        .Name = "Kid Friendly", 
        .Id = "kid_friendly", 
        .IsTimed = true
    }
);
