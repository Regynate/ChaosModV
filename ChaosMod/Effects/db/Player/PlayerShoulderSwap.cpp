#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR Cam shoulderCam = 0;

static void OnTick()
{
	auto const playerPed    = PLAYER_PED_ID();
	auto const aiming    = IS_PLAYER_FREE_AIMING(PLAYER_ID());

	auto const inVehicle = IS_PED_IN_ANY_VEHICLE(playerPed, false);

	if (!aiming || inVehicle)
	{
		RENDER_SCRIPT_CAMS(false, true, 250, true, true, 0);
		if (shoulderCam)
		{
			SET_CAM_ACTIVE(shoulderCam, false);
			DESTROY_CAM(shoulderCam, false);
			shoulderCam = 0;
		}
		return;
	}

	if (!DOES_CAM_EXIST(shoulderCam))
	{
		shoulderCam = CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
		RENDER_SCRIPT_CAMS(true, true, 250, true, true, 0);
	}

	if (shoulderCam != 0)
	{
		auto const rotationAngle = -30.f;

		auto const rotCoords     = GET_PED_BONE_COORDS(playerPed, 31086, 0.f, 0.f, 0.f);
		auto const ogRotation    = GET_GAMEPLAY_CAM_ROT(2);
		auto const ogPosition    = GET_GAMEPLAY_CAM_COORD();

		auto const ogVector      = ogPosition - rotCoords;

		auto const rotatedX      = ogVector.x * COS(rotationAngle) - ogVector.y * SIN(rotationAngle);
		auto const rotatedY      = ogVector.x * SIN(rotationAngle) + ogVector.y * COS(rotationAngle);
		auto const rotatedZ      = ogVector.z;

		auto const rotatedHeading = aiming ? GET_ENTITY_HEADING(playerPed) : ogRotation.z + rotationAngle;

		SET_CAM_COORD(shoulderCam, rotCoords.x + rotatedX, rotCoords.y + rotatedY, rotCoords.z + rotatedZ);
		SET_CAM_ROT(shoulderCam, ogRotation.x, ogRotation.y, rotatedHeading, 2);
		SET_CAM_FOV(shoulderCam, GET_GAMEPLAY_CAM_FOV());
		SET_CAM_ACTIVE(shoulderCam, true);
		SET_CAM_AFFECTS_AIMING(shoulderCam, false);
	}
}

static void OnStop()
{
	RENDER_SCRIPT_CAMS(false, false, 250, true, true, 0);
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
		.EffectCategory = EffectCategory::Camera
	}
);