#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static std::vector<std::int32_t> characters {};

static void OnStart()
{
	int playerHandle{};
	for (auto const ped : GetAllPeds())
	{
		auto const player     = PLAYER_PED_ID();
		if (ped == player) {
			playerHandle = player;
		
			continue;
		}
		auto const pedModel   = GET_ENTITY_MODEL(ped);

		auto const micheal    = "player_zero"_hash;
		auto const frankin    = "player_one"_hash;
		auto const trevor     = "player_two"_hash;

		if (pedModel == micheal || pedModel == frankin || pedModel == trevor)
		{
			auto const coords    = GET_ENTITY_COORDS(player, false);

			auto const pedCoords = GET_ENTITY_COORDS(ped, false);
			auto const switchType = GET_IDEAL_PLAYER_SWITCH_TYPE(coords.x, coords.y, coords.z, pedCoords.x, pedCoords.y, pedCoords.z);
			START_PLAYER_SWITCH(player, ped, 1024, switchType);
			return;
		}
	}
	
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Forced Switch (UNFINISHED)",
		.Id = "player_forced_switch",
		.IsTimed = false,
	}
);