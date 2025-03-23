#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static bool RequestControlEntity(Entity entity)
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
CHAOS_VAR auto constexpr PI               = 3.1415f;

static Vector3 GetAimingCoords()
{

	auto const player         = PLAYER_PED_ID();
	auto const cameraRotation = GET_GAMEPLAY_CAM_ROT(0);
	auto const rotation       = std::abs(std::cos(cameraRotation.x * degreesToRadians));

	Vector3 direction { -std::sin(cameraRotation.z * degreesToRadians) * rotation,
		                std::cos(cameraRotation.z * degreesToRadians) * rotation,
		                std::sin(cameraRotation.x * degreesToRadians) };

	auto start   = GET_GAMEPLAY_CAM_COORD();
	auto const end     = start + direction * 200.0f;

	auto const raycast =
	    START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(start.x, start.y, start.z, end.x, end.y, end.z, -1, player, 0);

	BOOL hit {};
	Vector3 hitCoordinates {};
	Vector3 surfaceNormal {};
	std::int32_t hitEntity {};

	GET_SHAPE_TEST_RESULT(raycast, &hit, &hitCoordinates, &surfaceNormal, &hitEntity);

	return hitCoordinates;
}

static Vector3 GetCameraDirection()
{
	auto const rotation   = GET_GAMEPLAY_CAM_ROT(2);
	auto const radiansX   = rotation.x * PI / 180.0f;
	auto const radiansZ   = rotation.z * PI / 180.0f;

	auto const directionX = -std::sin(radiansZ) * std::cos(radiansX);
	auto const directionY = std::cos(radiansZ) * std::cos(radiansX);
	auto const directionZ = std::sin(radiansX);

	return { directionX, directionY, directionZ };
}

static std::array<std::uint32_t, 5> animalModels = { GET_HASH_KEY("a_c_deer"), GET_HASH_KEY("a_c_coyote"),
	                                                 GET_HASH_KEY("a_c_boar"), GET_HASH_KEY("a_c_rabbit_01"),
	                                                 GET_HASH_KEY("a_c_mtlion") };

static void OnTick()
{
	auto const player = PLAYER_PED_ID();
	if (!IS_PED_SHOOTING(player))
		return;

	auto const aimingCoordinates = GetAimingCoords();
	auto const randomModel       = animalModels[GET_RANDOM_INT_IN_RANGE(0, animalModels.size() -1)];
	LoadModel(randomModel);

	auto constexpr rightHandBone = 57005;
	auto const handCoordinates   = GET_PED_BONE_COORDS(player, rightHandBone, 0.0f, 0.0f, 0.0f);
	auto const animal =
	    CreatePoolPed(28, randomModel, handCoordinates.x, handCoordinates.y, handCoordinates.z, 0.0f);

	SET_PED_TO_RAGDOLL(animal, 2000, 2000, 1, false, false, false);

	auto const aimDirection = GetCameraDirection();
	APPLY_FORCE_TO_ENTITY(animal, 1, aimDirection.x * 100.0f, aimDirection.y * 100.0f, aimDirection.z * 50.0f, 0, 0, 0,
	                      false, true, true, true, false, true);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Shoot Random Animals", 
        .Id = "misc_shoot_random_animals", 
        .IsTimed = true
    }
);
