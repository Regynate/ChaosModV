#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void SpawnBandMembers()
{
	static constexpr std::array<std::pair<const char *, const char *>, 6> characterList = {
		{ { "Lester", "cs_lestercrest" },
		  { "Brad", "cs_brad" },
		  { "Michael", "player_zero" },
		  { "Trevor", "player_two" },
		  { "Davey", "cs_dave_norton" },
		  { "Franklin", "player_one" } }
	};

	auto const playerPed       = PLAYER_PED_ID();
	auto const playerModelHash = GET_ENTITY_MODEL(playerPed);

	for (auto const &character : characterList)
	{
		auto const characterModelHash = GET_HASH_KEY(character.second);

		if (characterModelHash != playerModelHash)
		{
			auto const spawnCoords = GET_ENTITY_COORDS(playerPed, false);
			auto const bandMember = CreatePoolPed(4, characterModelHash, spawnCoords.x + GET_RANDOM_INT_IN_RANGE(-5, 5),
			                                      spawnCoords.y + GET_RANDOM_INT_IN_RANGE(-5, 5), spawnCoords.z, 0.0f);

			auto const isHostile  = GET_RANDOM_INT_IN_RANGE(0, 1);
			if (isHostile)
			{
				TASK_COMBAT_PED(bandMember, playerPed, 0, 16);
				SET_PED_KEEP_TASK(bandMember, true);
			}
			else
			{
				auto const playerGroup = GET_PLAYER_GROUP(playerPed);
				SET_PED_AS_GROUP_MEMBER(bandMember, playerGroup);
			}

			auto const weaponHash = GET_HASH_KEY("WEAPON_PISTOL");
			SET_PED_COMBAT_ABILITY(bandMember, 100);
			SET_PED_ACCURACY(bandMember, 100);
			SET_PED_CAN_SWITCH_WEAPON(bandMember, true);
			GIVE_WEAPON_TO_PED(bandMember, weaponHash, 9999, false, false);
		}
	}
}

static void OnStart()
{
	SpawnBandMembers();
}

static void OnTick()
{
}

static void OnStop()
{
}
// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr,
    {
        .Name = "We're Getting the Band Back Together",
        .Id = "peds_were_getting_the_band_back_together",
        .IsTimed = false
    }
);