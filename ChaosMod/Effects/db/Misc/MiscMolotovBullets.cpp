#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static constexpr float DEGREES_TO_RADIANS = 0.0174532924f;

static Vector3 GetAimingCoords()
{
	auto const player       = PLAYER_PED_ID();
	auto const cam_rotation = GET_GAMEPLAY_CAM_ROT(0);

	auto const rotation     = std::abs(std::cos(cam_rotation.x * DEGREES_TO_RADIANS));

	auto direction          = Vector3 { -std::sin(cam_rotation.z * DEGREES_TO_RADIANS) * rotation,
                               std::cos(cam_rotation.z * DEGREES_TO_RADIANS) * rotation,
                               std::sin(cam_rotation.x * DEGREES_TO_RADIANS) };

	auto start        = GET_GAMEPLAY_CAM_COORD();
	auto const end          = start + direction * 200.f;

	auto const raycast =
	    START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(start.x, start.y, start.z, end.x, end.y, end.z, -1, player, 0);

	BOOL hit {};
	Vector3 coords {};
	Vector3 surface {};
	std::int32_t entity {};

	GET_SHAPE_TEST_RESULT(raycast, &hit, &coords, &surface, &entity);

	return coords;
}

static void OnStart()
{
	auto const player      = PLAYER_PED_ID();
	auto const molotovHash = GET_HASH_KEY("weapon_molotov");
	GIVE_DELAYED_WEAPON_TO_PED(player, molotovHash, 1, false);
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	if (!IS_PED_SHOOTING(player))
		return;

	auto const molotovHash = GET_HASH_KEY("weapon_molotov");
	auto const coords      = GET_ENTITY_COORDS(player, false);
	auto const endCoords   = GetAimingCoords();

	SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords.x, coords.y, coords.z, endCoords.x, endCoords.y, endCoords.z, 100, 1,
	                                   molotovHash, player, 1, 1, 100.f);
}

static void OnStop()
{
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, OnTick, 
    {
        .Name = "Molotov Bullets", 
        .Id = "misc_molotov_bullets", 
        .IsTimed = true
    }
);
