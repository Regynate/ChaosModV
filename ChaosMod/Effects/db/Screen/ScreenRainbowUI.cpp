/*
    Effect by Gorakh
*/

#include <stdafx.h>

#include "Components/MetaModifiers.h"
#include "Effects/Register/RegisterEffect.h"

#include "Memory/UI.h"

static void OnTick()
{
	static ULONG cnt        = 0;
	static const float freq = .1f;

	if (++cnt >= (ULONG)-1)
		cnt = 0;

	if (ComponentExists<MetaModifiers>())
	{
		int r = std::lround(std::sin(freq * cnt) * 127 + 128);
		int g = std::lround(std::sin(freq * cnt + 2) * 127 + 128);
		int b = std::lround(std::sin(freq * cnt + 4) * 127 + 128);
		GetComponent<MetaModifiers>()->TimerColorOverride = { r, g, b };

		r = std::lround(std::sin(freq * cnt + 1002) * 127 + 128);
		g = std::lround(std::sin(freq * cnt + 1004) * 127 + 128);
		b = std::lround(std::sin(freq * cnt + 1006) * 127 + 128);
		GetComponent<MetaModifiers>()->EffectTextColorOverride = { r, g, b };

		r = std::lround(std::sin(freq * cnt + 3002) * 127 + 128);
		g = std::lround(std::sin(freq * cnt + 3004) * 127 + 128);
		b = std::lround(std::sin(freq * cnt + 3006) * 127 + 128);
		GetComponent<MetaModifiers>()->EffectTimerColorOverride = { r, g, b };
	}
}

static void OnStop()
{
	if (ComponentExists<MetaModifiers>())
	{
		GetComponent<MetaModifiers>()->TimerColorOverride       = { -1, -1, -1 };
		GetComponent<MetaModifiers>()->EffectTextColorOverride  = { -1, -1, -1 };
		GetComponent<MetaModifiers>()->EffectTimerColorOverride = { -1, -1, -1 };
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
	{
		.Name = "Rainbow Chaos UI",
		.Id = "screen_rainbow_ui",
		.IsTimed = true
	}
);