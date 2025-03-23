#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static std::int32_t feetCam = 0;

static void OnStart()
{
	auto const player = PLAYER_PED_ID();
	auto const coords = GET_ENTITY_COORDS(player, false);
	auto const SKEL_L_Foot = 14201;
	auto const footCoords  = GET_PED_BONE_COORDS(player, SKEL_L_Foot, 0, 0, 0);
	feetCam           = CREATE_CAM((char *)"DEFAULT_SCRIPTED_CAMERA", true);
	SET_CAM_COORD(feetCam, footCoords.x, footCoords.y, footCoords.z);
	SET_CAM_ROT(feetCam, 0.0f, 0.0f, GET_ENTITY_HEADING(player), 2);
	RENDER_SCRIPT_CAMS(true, false, 0, true, true, 0);
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	auto const coords = GET_ENTITY_COORDS(player, false);
	auto const SKEL_L_Foot = 14201;
	auto const footCoords  = GET_PED_BONE_COORDS(player, SKEL_L_Foot, 0, 0, 0);
	if (feetCam != 0)
	{
		SET_CAM_COORD(feetCam, footCoords.x, footCoords.y, footCoords.z);
		SET_CAM_ROT(feetCam, 0.0f, 0.0f, GET_ENTITY_HEADING(player), 2);
	}
}

static void OnStop()
{
	RENDER_SCRIPT_CAMS(false, false, 0, true, true, 0);
	if (feetCam != 0)
	{
		DESTROY_CAM(feetCam, false);
		feetCam = 0;
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Feet Cam",
        .Id = "misc_feet_cam", 
        .IsTimed = true
    }
);