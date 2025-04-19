#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static float DegreeToRadian(float degrees)
{
	auto constexpr PI = 3.1415f;
	return static_cast<float>((PI / 180) * degrees);
}

static Vector3 RotationToDirection(Vector3 rotation)
{
	auto radiansX  = DegreeToRadian(rotation.x);
	auto radiansZ  = DegreeToRadian(rotation.z);
	auto magnitude = std::abs(std::cos(radiansX));

	return { -std::sin(radiansZ) * magnitude, std::cos(radiansZ) * magnitude, std::sin(radiansX) };
}

static Vector3 GetCoordinatesInFront(Vector3 position, Vector3 rotation, float distance)
{
	auto direction = RotationToDirection(rotation);

	return { position.x + (direction.x * distance), position.y + (direction.y * distance),
		     position.z + (direction.z * distance) };
}

static void SpawnTheLostAndDamned()
{
	auto constexpr BF_CanLeaveVehicle = 3;

	auto const johnnyKlebitzModel = GET_HASH_KEY("cs_johnnyklebitz");
	auto const terryThorpeModel   = GET_HASH_KEY("cs_terry");
	auto const claySimmonsModel   = GET_HASH_KEY("cs_clay");
	auto const daemonModel        = GET_HASH_KEY("daemon");
	auto const hexerModel         = GET_HASH_KEY("hexer");
	auto const pistolModel        = GET_HASH_KEY("weapon_pistol");

	LoadModel(johnnyKlebitzModel);
	LoadModel(terryThorpeModel);
	LoadModel(claySimmonsModel);
	LoadModel(daemonModel);
	LoadModel(hexerModel);

	auto const player         = PLAYER_PED_ID();
	auto const playerCoordinates = GET_ENTITY_COORDS(player, false);
	auto const playerHeading  = GET_ENTITY_HEADING(player);
	auto const playerRotation = GET_ENTITY_ROTATION(player, 0);
	auto const behindPlayer = GetCoordinatesInFront(playerCoordinates, playerRotation, -2.0f);

	auto const johnnysBike = CreatePoolVehicle(hexerModel, behindPlayer.x, behindPlayer.y, behindPlayer.z, playerHeading);
	auto const terrysBike    = CreatePoolVehicle(daemonModel, behindPlayer.x - 2.0f, behindPlayer.y - 2.0f, behindPlayer.z, playerHeading);
	auto const claysBike  = CreatePoolVehicle(daemonModel, behindPlayer.x + 2.0f, behindPlayer.y - 2.0f, behindPlayer.z, playerHeading);

	auto const johnnyKlebitz = CreatePoolPedInsideVehicle(johnnysBike, 6, johnnyKlebitzModel, -1);
	auto const terryThorpe   = CreatePoolPedInsideVehicle(terrysBike, 6, terryThorpeModel, -1);
	auto const claySimmons   = CreatePoolPedInsideVehicle(claysBike, 6, claySimmonsModel, -1);

	auto const lostMcGroup   = CREATE_GROUP(0);
	SET_PED_AS_GROUP_LEADER(johnnyKlebitz, lostMcGroup);
	SET_PED_AS_GROUP_MEMBER(terryThorpe, lostMcGroup);
	SET_PED_AS_GROUP_MEMBER(claySimmons, lostMcGroup);

	SET_PED_NEVER_LEAVES_GROUP(terryThorpe, true);
	SET_PED_NEVER_LEAVES_GROUP(claySimmons, true);
	SET_PED_NEVER_LEAVES_GROUP(johnnyKlebitz, true);

	auto const lostMcRelGroup = GET_HASH_KEY("LOST");
	SET_PED_RELATIONSHIP_GROUP_HASH(johnnyKlebitz, lostMcRelGroup);
	SET_PED_RELATIONSHIP_GROUP_HASH(terryThorpe, lostMcRelGroup);
	SET_PED_RELATIONSHIP_GROUP_HASH(claySimmons, lostMcRelGroup);

	TASK_COMBAT_PED(johnnyKlebitz, player, 0, 16);
	TASK_COMBAT_PED(terryThorpe, player, 0, 16);
	TASK_COMBAT_PED(claySimmons, player, 0, 16);

	GIVE_DELAYED_WEAPON_TO_PED(johnnyKlebitz, pistolModel, 0, true);
	GIVE_DELAYED_WEAPON_TO_PED(terryThorpe, pistolModel, 0, true);
	GIVE_DELAYED_WEAPON_TO_PED(claySimmons, pistolModel, 0, true);

	SET_PED_COMBAT_ATTRIBUTES(johnnyKlebitz, BF_CanLeaveVehicle, false);
	SET_PED_COMBAT_ATTRIBUTES(terryThorpe, BF_CanLeaveVehicle, false);
	SET_PED_COMBAT_ATTRIBUTES(claySimmons, BF_CanLeaveVehicle, false);

	SET_VEHICLE_COLOURS(johnnysBike, 111, 64);
}

static void OnStart()
{
	SpawnTheLostAndDamned();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "The Lost and Damned", 
        .Id = "peds_the_lost_and_damned", 
        .IsTimed = false
    }
);
