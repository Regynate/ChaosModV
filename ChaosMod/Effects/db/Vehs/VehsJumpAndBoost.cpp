#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>
#include "Util/HelpText.h"

static void OnStart()
{
}

static void OnTick()
{
	DisplayHelpText("Jump = SPACE | Boost = X", 20);

	auto const player    = PLAYER_PED_ID();
	auto const inVehicle = IS_PED_IN_ANY_VEHICLE(player, false);
	if (!inVehicle)
		return;

	auto const vehicle        = GET_VEHICLE_PED_IS_IN(player, false);
	auto const isVehicleInAir = IS_ENTITY_IN_AIR(vehicle);

	if (GetAsyncKeyState(VK_SPACE) && !isVehicleInAir)
		APPLY_FORCE_TO_ENTITY(vehicle, 1, 0.0f, 0.0f, 25.0f, 0.0f, 0.0f, 0.0f, 0, true, true, true, false, false);
	auto constexpr X_KEY = 0x58;
	if (GetAsyncKeyState(X_KEY))
	{
		for (auto const _ : std::ranges::iota_view { 0, 150 })
		{
			ANIMPOSTFX_PLAY("RaceTurbo", 10000, true);

			auto const currentSpeed = GET_ENTITY_SPEED(vehicle);
			SET_VEHICLE_FORWARD_SPEED(vehicle, currentSpeed + 1);
			WAIT(0);
		}

		ANIMPOSTFX_STOP("RaceTurbo");

		while (IS_ENTITY_IN_AIR(vehicle))
			WAIT(100);
	}
}

static void OnStop()
{
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Jump And Boost", 
        .Id = "vehs_jump_and_boost", 
        .IsTimed = true
    }
);
