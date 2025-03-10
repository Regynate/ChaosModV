#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static std::int32_t feetCam = 0;

static void OnStart()
{
	auto const player = PLAYER_PED_ID();
	auto const coords = GET_ENTITY_COORDS(player, false);
	feetCam           = CREATE_CAM((char *)"DEFAULT_SCRIPTED_CAMERA", true);
	SET_CAM_COORD(feetCam, coords.x, coords.y, coords.z - 0.8f);
	SET_CAM_ROT(feetCam, 0.0f, 0.0f, GET_ENTITY_HEADING(player), 2);
	RENDER_SCRIPT_CAMS(true, false, 0, true, true, 0);
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	auto const coords = GET_ENTITY_COORDS(player, false);
	if (feetCam != 0)
	{
		SET_CAM_COORD(feetCam, coords.x, coords.y, coords.z - 0.8f);
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