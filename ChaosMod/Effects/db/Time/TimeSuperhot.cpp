/*
    Effect by Last0xygen
*/

#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR int lastCheck = 0;

static void OnStop()
{
	SET_TIME_SCALE(1.f);
}

static void OnTick()
{
	int current_time = GET_GAME_TIMER();
	if (current_time - lastCheck > 100)
	{
		lastCheck       = current_time;
		Ped playerPed   = PLAYER_PED_ID();
		float gameSpeed = 1;
		if (!IS_PED_GETTING_INTO_A_VEHICLE(playerPed) && !IS_PED_CLIMBING(playerPed) && !IS_PED_DIVING(playerPed)
		    && !IS_PED_JUMPING_OUT_OF_VEHICLE(playerPed) && !IS_PED_RAGDOLL(playerPed) && !IS_PED_GETTING_UP(playerPed))
		{
			float speed = GET_ENTITY_SPEED(playerPed);
			gameSpeed   = std::max(std::min(speed, 4.f) / 4.f, 0.2f);
		}
		SET_TIME_SCALE(gameSpeed);
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
	{
		.Name = "Superhot",
		.Id = "time_superhot",
		.IsTimed = true,
		.IncompatibleWith = { "player_dead_eye", "time_x02", "time_x05" }
	}
);