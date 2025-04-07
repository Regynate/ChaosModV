#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>
#include "Util/HelpText.h"

static float DegreeToRadian(float deg)
{
	auto constexpr PI = 3.1415f;
	const double rad  = (PI / 180) * deg;
	return (float)rad;
}

static Vector3 RotationToDirection(Vector3 rot)
{
	float x   = DegreeToRadian(rot.x);
	float z   = DegreeToRadian(rot.z);

	float num = abs(cos(x));

	return Vector3 { -sin(z) * num, cos(z) * num, sin(x) };
}

static Vector3 GetCoordsInFront(Vector3 pos, Vector3 rot, float dist)
{
	Vector3 ret {};

	float a = pos.x + (RotationToDirection(rot).x * dist);
	float b = pos.y + (RotationToDirection(rot).y * dist);
	float c = pos.z + (RotationToDirection(rot).z * dist);

	ret.x   = a;
	ret.y   = b;
	ret.z   = c;

	return ret;
}

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

static const std::array<std::uint32_t, 10> climbableObjects{
	GET_HASH_KEY("prop_logpile_05"), GET_HASH_KEY("prop_pipes_conc_01"), GET_HASH_KEY("prop_cablespool_05"), GET_HASH_KEY("prop_generator_03a"),
	GET_HASH_KEY("prop_woodpile_04b"), GET_HASH_KEY("prop_cons_ply01"), GET_HASH_KEY("prop_conc_blocks01c"), GET_HASH_KEY("prop_pipes_01b"),
	GET_HASH_KEY("prop_drywallpile_01"), GET_HASH_KEY("prop_cementbags01")
};
CHAOS_VAR std::vector<Entity> spawnedObjects {};
static void OnStop()
{
	if (spawnedObjects.empty())
		return;

	for (auto const &objects : spawnedObjects)
	{
		DeleteEntity(objects);
	}

	spawnedObjects.clear();
}

static void OnTick()
{
	DisplayHelpText("Press: H");

	auto constexpr KEY_H = 0x48;
	if (GetAsyncKeyState(KEY_H))
	{

		auto const player          = PLAYER_PED_ID();
		auto const rotation        = GET_ENTITY_ROTATION(player, 0);
		auto const coords          = GET_ENTITY_COORDS(player, false);
		auto const inFront         = GetCoordsInFront(coords, rotation, 3);
		auto const heading         = GET_ENTITY_HEADING(player);

		auto const max             = climbableObjects.size();
		auto const randomIteration = GET_RANDOM_INT_IN_RANGE(0, max);
		auto const selectedObject  = climbableObjects[randomIteration];

		LoadModel(selectedObject);
		auto const prop = CreatePoolProp(selectedObject, inFront.x, inFront.y, inFront.z - 1, heading);
		FREEZE_ENTITY_POSITION(prop, true);
		spawnedObjects.emplace_back(prop);

		WAIT(500);
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
	{
		.Name = "Bob The Builder",
		.Id = "player_bob_the_builder",
		.IsTimed = true,
	}
);