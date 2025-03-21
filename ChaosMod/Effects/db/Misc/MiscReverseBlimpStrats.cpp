#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static float DegreeToRadian(float degrees)
{
	auto constexpr PI = 3.1415f;
	return static_cast<float>((PI / 180) * degrees);
}

static Vector3 RotationToDirection(Vector3 rotation)
{
	auto const x         = DegreeToRadian(rotation.x);
	auto const z         = DegreeToRadian(rotation.z);
	auto const magnitude = std::abs(std::cos(x));

	return { -std::sin(z) * magnitude, std::cos(z) * magnitude, std::sin(x) };
}

static Vector3 GetCoordinatesInFront(Vector3 position, Vector3 rotation, float distance)
{
	auto const direction = RotationToDirection(rotation);
	return { position.x + (direction.x * distance), position.y + (direction.y * distance),
		     position.z + (direction.z * distance) };
}

static void OnStart()
{
	auto const player            = PLAYER_PED_ID();
	auto const playerCoordinates = GET_ENTITY_COORDS(player, false);
	auto const playerRotation    = GET_ENTITY_ROTATION(player, 0);
	auto const playerHeading     = GET_ENTITY_HEADING(player);
	auto const behindPosition    = GetCoordinatesInFront(playerCoordinates, playerRotation, -155.0f);

	auto const blimpModel        = GET_HASH_KEY("blimp");
	LoadModel(blimpModel);
	auto const blimp = CreatePoolVehicle(blimpModel, behindPosition.x, behindPosition.y, behindPosition.z + 25.0f,
	                                  playerHeading);

	CREATE_RANDOM_PED_AS_DRIVER(blimp, false);

	auto const daveyModel = GET_HASH_KEY("cs_davenorton");
	LoadModel(daveyModel);
	auto const davey =
	    CREATE_PED(6, daveyModel, behindPosition.x, behindPosition.y, behindPosition.z - 20.0f, 0.0f, false, true);

	SET_VEHICLE_FORWARD_SPEED(blimp, 50.0f);

	WAIT(2000);

	SET_PED_INTO_VEHICLE(davey, blimp, 0);
	TASK_LEAVE_VEHICLE(davey, blimp, 4160);

	WAIT(2000);

	while (IS_ENTITY_IN_AIR(davey))
	{
		SET_ENTITY_INVINCIBLE(davey, true);
		WAIT(1000);
	}

	SET_ENTITY_INVINCIBLE(davey, false);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Inverse Blimp Strats", 
        .Id = "misc_inverse_blimp_strats", 
        .IsTimed = false
    }
);
