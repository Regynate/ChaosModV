#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Util/HelpText.h"

static void OnTick()
{
	static bool hasJumped  = false;
	static bool hasBoosted = false;

	auto const player      = PLAYER_PED_ID();
	auto const inVehicle   = IS_PED_IN_ANY_VEHICLE(player, false);
	if (!inVehicle)
	{
		hasBoosted = hasJumped = false;
		return;
	}

	auto const vehicle        = GET_VEHICLE_PED_IS_IN(player, false);
	auto const isVehicleInAir = IS_ENTITY_IN_AIR(vehicle);
	
	if (!isVehicleInAir)
	{
		hasJumped  = false;
		hasBoosted = false;
		DisplayHelpText("Jump = ~INPUT_VEH_JUMP~");

		if (IS_CONTROL_JUST_PRESSED(0, 102))
		{
			APPLY_FORCE_TO_ENTITY(vehicle, 1, 0.0f, 0.0f, 60.0f, 0.0f, 0.0f, 0.0f, 0, true, true, true, false, false);
			hasJumped = true;
		}
	}

	if (hasJumped && !hasBoosted)
	{
		DisplayHelpText("Boost = ~INPUT_VEH_DROP_PROJECTILE~");

		if (IS_CONTROL_JUST_PRESSED(0, 105) && hasJumped && !hasBoosted)
		{
			auto startTick = GET_GAME_TIMER();

			while (GET_GAME_TIMER() - startTick < 1000)
			{
				ANIMPOSTFX_PLAY("RaceTurbo", 10000, true);

				auto const currentSpeed = GET_ENTITY_SPEED(vehicle);
				SET_VEHICLE_FORWARD_SPEED(vehicle, currentSpeed + GET_FRAME_TIME() * 50);
				WAIT(0);
			}

			ANIMPOSTFX_STOP("RaceTurbo");
			hasBoosted = true;
		}
	}
}

// clang-format off
	REGISTER_EFFECT(nullptr, nullptr, OnTick, 
		{
        .Name = "Jump And Boost", 
        .Id = "vehs_jump_and_boost", 
        .IsTimed = true
    }
);
