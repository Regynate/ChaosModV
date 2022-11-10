/*
    Effect by OnlyRealNubs, modified
*/

#include <stdafx.h>

static void OnTick()
{
	static auto lastTickToDraw = 0;
	auto curTick               = GetTickCount64();

	LOG("" << curTick << " " << lastTickToDraw);
	if (curTick < lastTickToDraw)
	{
		DRAW_RECT(0, 0, 2.f, 2.f, 255, 255, 255, 255, 0);
	}

	if (curTick >= lastTickToDraw + 5000)
	{
		lastTickToDraw = curTick + 3000;
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, EffectInfo { 
		.Name = "Sticky Flashbang", 
		.Id = "misc_sticky_flashbang",
		.IsTimed = true,
		.IsShortDuration = true
	}
);