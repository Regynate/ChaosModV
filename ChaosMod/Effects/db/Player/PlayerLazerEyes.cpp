#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static bool RequestControlEntity(const std::uint32_t entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(std::int32_t entity)
{
	if (!RequestControlEntity(entity))
		return;
	if (!IS_ENTITY_A_MISSION_ENTITY(entity))
		SET_ENTITY_AS_MISSION_ENTITY(entity, true, true);
	DELETE_ENTITY(&entity);
}

static auto constexpr degreesToRadians = 0.0174532924f;

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
	std::int32_t entity {};

	GET_SHAPE_TEST_RESULT(raycast, &hit, &coords, &surface, &entity);

	return coords;
}

static void DeleteHitEntity()
{
	auto const player       = PLAYER_PED_ID();
	auto const playerCoords = GET_ENTITY_COORDS(player, false);
	auto const aimingCoords = GetAimingCoords();

	auto const raycast      = START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(
        playerCoords.x, playerCoords.y, playerCoords.z, aimingCoords.x,
	                                                   aimingCoords.y, aimingCoords.z, -1, player, 0);

	BOOL hit {};
	Vector3 hitCoords {};
	Vector3 surface {};
	std::int32_t hitEntity {};

	GET_SHAPE_TEST_RESULT(raycast, &hit, &hitCoords, &surface, &hitEntity);
	
	if (aimingCoords.x == 0 && aimingCoords.y == 0 && aimingCoords.z == 0)
	{
		return;
	}

	DRAW_LINE(playerCoords.x, playerCoords.y, playerCoords.z + 0.8, aimingCoords.x, aimingCoords.y, aimingCoords.z, 255,0, 0, 255);


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
