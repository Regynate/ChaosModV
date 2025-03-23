#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>
static float DegreeToRadian(const float degrees)
{
	auto constexpr PI = 3.1415f;
	return static_cast<float>((PI / 180) * degrees);
}

static Vector3 RotationToDirection(const Vector3 rotation)
{
	auto const radiansX  = DegreeToRadian(rotation.x);
	auto const radiansZ  = DegreeToRadian(rotation.z);
	auto const magnitude = std::abs(std::cos(radiansX));

	return { -std::sin(radiansZ) * magnitude, std::cos(radiansZ) * magnitude, std::sin(radiansX) };
}

static Vector3 GetCoordinatesInFront(const Vector3 position, const Vector3 rotation, const float distance)
{
	auto const direction = RotationToDirection(rotation);

	return { position.x + (direction.x * distance), position.y + (direction.y * distance),
		     position.z + (direction.z * distance) };
}

static void PedsShootSnowballs()
{
	for (auto const ped : GetAllPeds())
	{
		if (!IS_PED_SHOOTING(ped))
			return;

		auto const snowballHash   = GET_HASH_KEY("weapon_snowball");
		auto const pedCoordinates = GET_ENTITY_COORDS(ped, false);
		auto const pedRotation    = GET_ENTITY_ROTATION(ped, 0);
		auto const endCoordinates = GetCoordinatesInFront(pedCoordinates, pedRotation, 10.0f);

		SHOOT_SINGLE_BULLET_BETWEEN_COORDS(pedCoordinates.x, pedCoordinates.y, pedCoordinates.z, endCoordinates.x,
		                                   endCoordinates.y, endCoordinates.z, 100, true, snowballHash, ped, true,
		                                   true, 100.0f);
	}
}

static void OnStart()
{
	auto const player       = PLAYER_PED_ID();
	auto const snowballHash = GET_HASH_KEY("weapon_snowball");
	GIVE_DELAYED_WEAPON_TO_PED(player, snowballHash, 1, false);
}

static void OnTick()
{
	PedsShootSnowballs();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, OnTick, 
    {
        .Name = "Snowball Fight", 
        .Id = "misc_snowball_fight", 
        .IsTimed = true
    }
);
