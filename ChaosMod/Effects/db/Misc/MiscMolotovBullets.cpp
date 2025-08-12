#include <stdafx.h>

#include "Memory/Weapon.h"
#include "Effects/Register/RegisterEffect.h"

static constexpr float DEGREES_TO_RADIANS = 0.0174532924f;

static Vector3 GetAimingCoords()
{
	auto const player       = PLAYER_PED_ID();
	auto const cam_rotation = GET_GAMEPLAY_CAM_ROT(0);

	auto const rotation     = std::abs(std::cos(cam_rotation.x * DEGREES_TO_RADIANS));

	auto direction          = Vector3 { -std::sin(cam_rotation.z * DEGREES_TO_RADIANS) * rotation,
                               std::cos(cam_rotation.z * DEGREES_TO_RADIANS) * rotation,
                               std::sin(cam_rotation.x * DEGREES_TO_RADIANS) };

	auto start              = GET_GAMEPLAY_CAM_COORD();
	auto const end          = start + direction * 200.f;

	auto const raycast =
	    START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(start.x, start.y, start.z, end.x, end.y, end.z, -1, player, 0);

	BOOL hit {};
	Vector3 coords {};
	Vector3 surface {};
	Entity entity {};

	GET_SHAPE_TEST_RESULT(raycast, &hit, &coords, &surface, &entity);

	return hit ? coords : Vector3();
}

static void OnStart()
{
	auto const player      = PLAYER_PED_ID();
	auto const molotovHash = GET_HASH_KEY("weapon_rpg");
	GIVE_DELAYED_WEAPON_TO_PED(player, molotovHash, 1, false);
}

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	if (!IS_PED_SHOOTING(player))
		return;

	auto const molotovHash = GET_HASH_KEY("weapon_rpg");
	auto const coords      = GET_ENTITY_COORDS(player, false);
	auto const cameraRot   = GET_GAMEPLAY_CAM_ROT(2);

	auto endCoords         = GetAimingCoords();
	if (endCoords.IsDefault())
	{
		const float distance = 20.f;
		Vector3 direction;
		direction.x = COS(cameraRot.x) * SIN(-cameraRot.z) * distance;
		direction.y = COS(cameraRot.x) * COS(-cameraRot.z) * distance;
		direction.z = SIN(cameraRot.x) * distance;

		endCoords   = direction + coords;
	}

	SHOOT_SINGLE_BULLET_BETWEEN_COORDS(coords.x, coords.y, coords.z, endCoords.x, endCoords.y, endCoords.z, 100, 1,
	                                   molotovHash, player, 1, 1, 5000.f);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, OnTick, 
    {
        .Name = "RPG Bullets", 
        .Id = "misc_rpg_bullets", 
        .IsTimed = true
    }
);
