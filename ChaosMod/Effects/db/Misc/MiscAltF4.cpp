#include <stdafx.h>


#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_MENU;
    inputs[0].ki.wScan = MapVirtualKey(VK_MENU, MAPVK_VK_TO_VSC);
    inputs[0].ki.dwFlags = 0;
    inputs[0].ki.time = 0;
    inputs[0].ki.dwExtraInfo = 0;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_F4;
    inputs[1].ki.wScan = MapVirtualKey(VK_F4, MAPVK_VK_TO_VSC);
    inputs[1].ki.dwFlags = 0;
    inputs[1].ki.time = 0;
    inputs[1].ki.dwExtraInfo = 0;

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_MENU;
    inputs[2].ki.wScan = MapVirtualKey(VK_MENU, MAPVK_VK_TO_VSC);
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[2].ki.time = 0;
    inputs[2].ki.dwExtraInfo = 0;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_F4;
    inputs[3].ki.wScan = MapVirtualKey(VK_F4, MAPVK_VK_TO_VSC);
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].ki.time = 0;
    inputs[3].ki.dwExtraInfo = 0;

	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr,
	{
		.Name = "Alt+F4",
		.Id = "misc_alt_f4"
	}
);