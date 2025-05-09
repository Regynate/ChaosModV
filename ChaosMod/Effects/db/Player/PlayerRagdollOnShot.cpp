#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Memory/WeaponPool.h"

static void OnTick()
{
	Ped playerPed = PLAYER_PED_ID();

	int curTime   = GET_GAME_TIMER();

	for (Hash weapon : Memory::GetAllWeapons())
	{
		int timeSinceDmg = _GET_TIME_OF_LAST_PED_WEAPON_DAMAGE(playerPed, weapon);
		if (timeSinceDmg && curTime - timeSinceDmg < 200)
		{
			if (IS_PED_IN_ANY_VEHICLE(playerPed, false))
				CLEAR_PED_TASKS_IMMEDIATELY(playerPed);

			SET_PED_TO_RAGDOLL(playerPed, 500, 1000, 0, true, true, false);

			CREATE_NM_MESSAGE(true, 0);
			GIVE_PED_NM_MESSAGE(playerPed);

			break;
		}
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
	{
		.Name = "Player Ragdolls When Shot",
		.Id = "player_ragdollondmg",
		.IsTimed = true,
		.IncompatibleWith = { "player_noragdoll" }
	}
);