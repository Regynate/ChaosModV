#include <stdafx.h>

#include "KeyStates.h"

void KeyStates::OnRun()
{
	for (BYTE i = 0; i < std::numeric_limits<BYTE>::max(); i++)
	{
		m_KeyStates[i] = GetAsyncKeyState(i)
		                   ? m_KeyStates[i] != KeyState::Released ? KeyState::Pressed : KeyState::JustPressed
		                   : KeyState::Released;
	}
}

bool KeyStates::IsKeyPressed(BYTE key) const
{
	return m_KeyStates[key] != KeyState::Released;
}

bool KeyStates::IsKeyJustPressed(BYTE key) const
{
	return m_KeyStates[key] == KeyState::JustPressed;
}

void KeyStates::SimulateKeyPress(BYTE key) const
{
	INPUT inputs[1]    = {};
	inputs[0].type     = INPUT_KEYBOARD;
	inputs[0].ki.wVk   = key;
	inputs[0].ki.wScan = MapVirtualKey(key, MAPVK_VK_TO_VSC);
	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void KeyStates::SimulateKeyRelease(BYTE key) const
{
	INPUT inputs[1]      = {};
	inputs[0].type       = INPUT_KEYBOARD;
	inputs[0].ki.wVk     = key;
	inputs[0].ki.wScan   = MapVirtualKey(key, MAPVK_VK_TO_VSC);
	inputs[0].ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}