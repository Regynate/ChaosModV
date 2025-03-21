#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void OnStart()
{
}

static void OnTick()
{
	INPUT inputs[2] = {};
	ZeroMemory(inputs, sizeof(inputs));

	constexpr uint code      = 0x53;
	inputs[0].type           = INPUT_KEYBOARD;
	inputs[0].ki.wScan       = MapVirtualKey(code, MAPVK_VK_TO_VSC);
	inputs[0].ki.time        = 0;
	inputs[0].ki.dwExtraInfo = 0;
	inputs[0].ki.wVk         = code;
	inputs[0].ki.dwFlags     = 0;

	inputs[1].type           = INPUT_KEYBOARD;
	inputs[1].ki.wVk         = VK_GAMEPAD_LEFT_THUMBSTICK_UP;

	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
	WAIT(50);
}

static void OnStop()
{

	INPUT inputs[1] = {};
	ZeroMemory(inputs, sizeof(inputs));

	constexpr uint code      = 0x53;
	inputs[0].type           = INPUT_KEYBOARD;
	inputs[0].ki.wScan       = MapVirtualKey(code, MAPVK_VK_TO_VSC);
	inputs[0].ki.time        = 0;
	inputs[0].ki.dwExtraInfo = 0;
	inputs[0].ki.wVk         = code;
	inputs[0].ki.dwFlags     = KEYEVENTF_KEYUP;

	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
    {
        .Name = "Help My S Key Is Stuck", 
        .Id = "misc_help_my_s_key_is_stuck", 
        .IsTimed = true,
		.IsShortDuration = true
    }
);
