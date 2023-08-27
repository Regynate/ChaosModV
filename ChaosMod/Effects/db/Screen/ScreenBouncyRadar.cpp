#include <stdafx.h>

#include "Memory/UI.h"

static float xOffset, yOffset, xVelocityBase, yVelocityBase, xVelocity, yVelocity, xMin, xMax, yMin, yMax;

static int lastTick;

static void OnStart()
{
	int safezoneSize = GET_PROFILE_SETTING(212);
	int screenWidth, screenHeight;
	_GET_ACTIVE_SCREEN_RESOLUTION(&screenWidth, &screenHeight);

	float multiplier = ((float)screenWidth / (float)screenHeight) * (9.f / 16);

	if (multiplier > 1)
	{
		xMin = -0.005f * safezoneSize * multiplier - (multiplier - 1) / 2;
	}
	else
	{
		xMin = -0.005f * safezoneSize * multiplier;
	}
	xMax      = multiplier - 0.14f + xMin;
	yMax      = 0.005f * safezoneSize;
	yMin      = yMax - 0.825f;

	xOffset   = 0.f;
	yOffset   = 0.f;

	xVelocityBase = g_Random.GetRandomFloat(0.5f, 1.f);
	yVelocityBase = g_Random.GetRandomFloat(0.5f, 1.f);

	xVelocity     = xVelocityBase * (g_Random.GetRandomInt(0, 1) ? -1 : 1);
	yVelocity     = yVelocityBase * (g_Random.GetRandomInt(0, 1) ? -1 : 1);

	lastTick  = GET_GAME_TIMER();
}

static void OnTick()
{
	int curTick = GET_GAME_TIMER();

	if (xOffset < xMin)
	{
		xVelocity = xVelocityBase;
	}
	else if (xOffset > xMax)
	{
		xVelocity = -xVelocityBase;
	}

	if (yOffset < yMin)
	{
		yVelocity = yVelocityBase;
	}
	else if (yOffset > yMax)
	{
		yVelocity = -yVelocityBase;
	}

	xOffset += xVelocity * (curTick - lastTick) / 2000;
	yOffset += yVelocity * (curTick - lastTick) / 2000;

	Memory::SetRadarOffset(xOffset, yOffset);

	lastTick = curTick;
}

static void OnStop()
{
	Memory::ResetRadar();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, EffectInfo
	{
		.Name = "Bouncy Radar",
		.Id = "screen_bouncyradar",
		.IsTimed = true,
		.IncompatibleWith = { "no_hud", "screen_maximap", "no_radar" }
	}
);