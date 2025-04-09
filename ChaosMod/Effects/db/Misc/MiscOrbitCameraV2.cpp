#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR int orbitCamera               = 0;
CHAOS_VAR float angle                = 0.0f;
CHAOS_VAR constexpr float rotationSpeed = 0.5f;
CHAOS_VAR constexpr float orbitDistance = 5.0f;

static float DEG_TO_RAD(float degrees) 
{
	auto constexpr PI = 3.1415f;
	return degrees * (PI / 180.0f);
};

static void CreateOrbitCamera()
{
	auto const player       = PLAYER_PED_ID();
	auto const playerCoords = GET_ENTITY_COORDS(player, true);

	orbitCamera             = CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
	SET_CAM_ACTIVE(orbitCamera, true);
	RENDER_SCRIPT_CAMS(true, false, 3000, true, false, 0);
}

static void DestroyOrbitCamera()
{
	if (DOES_CAM_EXIST(orbitCamera))
	{
		RENDER_SCRIPT_CAMS(false, false, 3000, true, false, 0);
		DESTROY_CAM(orbitCamera, false);
	}
}

static void UpdateOrbitCamera()
{
	auto const player       = PLAYER_PED_ID();
	auto const playerCoords = GET_ENTITY_COORDS(player, true);

	angle += rotationSpeed;
	if (angle > 360.0f)
		angle -= 360.0f;

	auto const radianAngle = DEG_TO_RAD(angle);
	auto const camX        = playerCoords.x + orbitDistance * cos(radianAngle);
	auto const camY        = playerCoords.y + orbitDistance * sin(radianAngle);
	auto const camZ        = playerCoords.z + 2.0f;

	SET_CAM_COORD(orbitCamera, camX, camY, camZ);
	POINT_CAM_AT_ENTITY(orbitCamera, player, 0.0f, 0.0f, 0.0f, false);
}

static void OnStart()
{
	CreateOrbitCamera();
}

static void OnStop()
{
	DestroyOrbitCamera();
}

static void OnTick()
{
	UpdateOrbitCamera();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Orbit Camera V2", 
        .Id = "misc_orbit_camera_v2", 
        .IsTimed = true,
		.IsShortDuration = true
    }
);