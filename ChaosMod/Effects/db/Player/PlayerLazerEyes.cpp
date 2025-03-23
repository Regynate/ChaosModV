#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static bool RequestControlEntity(const Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(Entity entity)
{
	if (!RequestControlEntity(entity))
		return;
	if (!IS_ENTITY_A_MISSION_ENTITY(entity))
		SET_ENTITY_AS_MISSION_ENTITY(entity, true, true);
	DELETE_ENTITY(&entity);
}

CHAOS_VAR auto constexpr degreesToRadians = 0.0174532924f;

static Vector3 AddVector(Vector3 vector_a, Vector3 vector_b)
{
	Vector3 result;
	result.x = vector_a.x;
	result.y = vector_a.y;
	result.z = vector_a.z;
	result.x += vector_b.x;
	result.y += vector_b.y;
	result.z += vector_b.z;
	return result;
}

static Vector3 multiplyVector(Vector3 vector, float x)
{
	Vector3 result;
	result.x = vector.x;
	result.y = vector.y;
	result.z = vector.z;
	result.x *= x;
	result.y *= x;
	result.z *= x;
	return result;
}

static float DegreeToRadian(float deg)
{
	const double rad = (3.14159265359f / 180) * deg;
	return (float)rad;
}

static Vector3 RotationToDirection(Vector3 rot)
{
	float x   = DegreeToRadian(rot.x);
	float z   = DegreeToRadian(rot.z);

	float num = abs(cos(x));

	return Vector3 { -sin(z) * num, cos(z) * num, sin(x) };
}

static void DrawLine()
{
auto const player             = PLAYER_PED_ID();
	auto const playerCoords       = GET_ENTITY_COORDS(player, false);

	auto const rotation           = GET_GAMEPLAY_CAM_ROT(0);
	auto const direction          = RotationToDirection(rotation);
	auto const startPosition      = AddVector(GET_GAMEPLAY_CAM_COORD(), multiplyVector(direction, 1.f));
	auto const endPosition        = AddVector(GET_GAMEPLAY_CAM_COORD(), multiplyVector(direction, 9999.f));

	auto const player_head_coords = PED::GET_PED_BONE_COORDS(player, 0x796e, 0, 0, 0);

	DRAW_LINE(player_head_coords.x, player_head_coords.y, player_head_coords.z, endPosition.x, endPosition.y,
	          endPosition.z, 255, 0, 0, 200);
}

static Vector3 GetAimingCoords()
{
	auto const player      = PLAYER_PED_ID();
	auto const camRotation = GET_GAMEPLAY_CAM_ROT(0);
	auto const rotation    = std::abs(std::cos(camRotation.x * degreesToRadians));

	Vector3 direction { -std::sin(camRotation.z * degreesToRadians) * rotation,
		                std::cos(camRotation.z * degreesToRadians) * rotation,
		                std::sin(camRotation.x * degreesToRadians) };

	auto start   = GET_GAMEPLAY_CAM_COORD();
	auto const end     = start + direction * 200.0f;

	auto const raycast =
	    START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(start.x, start.y, start.z, end.x, end.y, end.z, -1, player, 0);

	BOOL hit {};
	Vector3 coords {};
	Vector3 surface {};
	Entity entity {};

	GET_SHAPE_TEST_RESULT(raycast, &hit, &coords, &surface, &entity);

	return coords;
}

static void DeleteHitEntity()
{
	auto const player       = PLAYER_PED_ID();
	auto const playerCoords = GET_ENTITY_COORDS(player, false);
	auto const aimingCoords = GetAimingCoords();

	DrawLine();

	auto const raycast      = START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(
        playerCoords.x, playerCoords.y, playerCoords.z, aimingCoords.x,
	                                                   aimingCoords.y, aimingCoords.z, -1, player, 0);

	BOOL hit {};
	Vector3 hitCoords {};
	Vector3 surface {};
	Entity hitEntity {};

	GET_SHAPE_TEST_RESULT(raycast, &hit, &hitCoords, &surface, &hitEntity);
	
	if (aimingCoords.x == 0 && aimingCoords.y == 0 && aimingCoords.z == 0)
	{
		return;
	}

	DeleteEntity(hitEntity);
	SET_ENTITY_COORDS(hitEntity, 0, 0, 0, false, false, false, true);
}

static void OnTick()
{
	DeleteHitEntity();
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Laser Eyes", 
        .Id = "player_laser_eyes", 
        .IsTimed = true
    }
);
