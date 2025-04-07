#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR Cam shoulderCam = 0;

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	auto const coords = GET_ENTITY_COORDS(player, false);
	auto const aiming = IS_PLAYER_FREE_AIMING(PLAYER_ID());

	auto const inVehicle = IS_PED_IN_ANY_VEHICLE(player, false);
	if (inVehicle)
		return;

	if (!aiming)
	{
		RENDER_SCRIPT_CAMS(false, true, 250, true, true, 0);
		SET_CAM_ACTIVE(shoulderCam, false);
		DESTROY_CAM(shoulderCam, false);
		shoulderCam = 0;
		return;
	}

	if (!DOES_CAM_EXIST(shoulderCam))
	{
		shoulderCam = CREATE_CAM((char *)"DEFAULT_SCRIPTED_CAMERA", true);
		RENDER_SCRIPT_CAMS(true, true, 250, true, true, 0);
	}

	if (shoulderCam != 0)
	{
		float heading    = GET_ENTITY_HEADING(player);
		float headingRad = heading * (3.14159f / 180.0f);
		float offsetX    = -0.5f * cos(headingRad);
		float offsetY    = -0.5f * sin(headingRad);
		float offsetZ    = 0.6f;
		SET_CAM_COORD(shoulderCam, coords.x + offsetX, coords.y + offsetY, coords.z + offsetZ);
		SET_CAM_ROT(shoulderCam, 0.0f, 0.0f, heading, 2);
		SET_CAM_ACTIVE(shoulderCam, true);
	}
}

static void OnStop()
{
	RENDER_SCRIPT_CAMS(false, false, 0, true, true, 0);
	if (shoulderCam != 0)
	{
		DESTROY_CAM(shoulderCam, false);
		shoulderCam = 0;
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
	{
		.Name = "Shoulder Swap",
		.Id = "player_shoulder_swap",
		.IsTimed = true,
	}
);