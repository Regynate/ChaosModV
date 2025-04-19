#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	auto constexpr COMPLETE = 12;

	static auto const micheal = GET_HASH_KEY("player_zero");
	static auto const frankin = GET_HASH_KEY("player_one");
	static auto const trevor  = GET_HASH_KEY("player_two");
	static auto const brad  = GET_HASH_KEY("ig_brad");

	for (auto const ped : GetAllPeds())
	{
		auto const player     = PLAYER_PED_ID();
		if (ped == player) 
			continue;

		auto const pedModel   = GET_ENTITY_MODEL(ped);

		if (pedModel == micheal || pedModel == frankin || pedModel == trevor || pedModel == brad)
		{
			auto const coords    = GET_ENTITY_COORDS(player, false);
			auto const pedCoords = GET_ENTITY_COORDS(ped, false);
			auto const switchType = GET_IDEAL_PLAYER_SWITCH_TYPE(coords.x, coords.y, coords.z, pedCoords.x, pedCoords.y, pedCoords.z);
			
			START_PLAYER_SWITCH(player, ped, 1024, switchType);
			
			while (GET_PLAYER_SWITCH_STATE() != COMPLETE)
				WAIT(1);

			CHANGE_PLAYER_PED(PLAYER_ID(), ped, false, true);
			return;
		}
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Switch To Different Character",
		.Id = "player_forced_switch",
		.IsTimed = false,
	}
);