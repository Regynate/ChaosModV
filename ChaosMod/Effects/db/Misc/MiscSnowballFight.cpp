#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void ped(void (*processor)(const std::int32_t))
{
	static constexpr std::int32_t MAX_ENTITIES = 100;
	std::int32_t peds[MAX_ENTITIES];
	auto const pedCount = worldGetAllPeds(peds, MAX_ENTITIES);

	for (auto const i : std::ranges::iota_view { 0, pedCount })
	{
		auto const pedHandle = peds[i];

		if (!DOES_ENTITY_EXIST(pedHandle))
			continue;

		processor(pedHandle);
	}
}

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

static void PedsShootSnowballs(const std::int32_t pedHandle)
{
	if (!IS_PED_SHOOTING(pedHandle))
		return;

	auto const snowballHash   = GET_HASH_KEY("weapon_snowball");
	auto const pedCoordinates = GET_ENTITY_COORDS(pedHandle, false);
	auto const pedRotation    = GET_ENTITY_ROTATION(pedHandle, 0);
	auto const endCoordinates = GetCoordinatesInFront(pedCoordinates, pedRotation, 10.0f);

	SHOOT_SINGLE_BULLET_BETWEEN_COORDS(pedCoordinates.x, pedCoordinates.y, pedCoordinates.z, endCoordinates.x,
	                                   endCoordinates.y, endCoordinates.z, 100, true, snowballHash, pedHandle, true,
	                                   true, 100.0f);
}

static void OnStart()
{
	auto const player       = PLAYER_PED_ID();
	auto const snowballHash = GET_HASH_KEY("weapon_snowball");
	GIVE_DELAYED_WEAPON_TO_PED(player, snowballHash, 1, false);
}

static void OnTick()
{
	ped(PedsShootSnowballs);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, OnTick, 
    {
        .Name = "Snowball Fight", 
        .Id = "misc_snowball_fight", 
        .IsTimed = true
    }
);
