#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>
#include <ranges>

static void OnTick()
{
	auto constexpr INPUT_MOVE_UD               = 31;
	auto constexpr INPUT_VEH_BRAKE             = 72;
	auto constexpr INPUT_VEH_FLY_THROTTLE_DOWN = 88;

	_SET_CONTROL_NORMAL(0, INPUT_MOVE_UD, 1.0f);
	_SET_CONTROL_NORMAL(0, INPUT_VEH_FLY_THROTTLE_DOWN, 1.0f);
	_SET_CONTROL_NORMAL(0, INPUT_VEH_BRAKE, 1.0f);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Help My S Key Is Stuck", 
        .Id = "misc_help_my_s_key_is_stuck", 
        .IsTimed = true,
		.IsShortDuration = true
    }
);
