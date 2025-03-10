#include <stdafx.h>
#include "Effects/Register/RegisterEffect.h"

static void allobject(void (*processor)(std::int32_t))
{
	static constexpr std::int32_t MAX_ENTITIES = 100;
	std::int32_t objects[MAX_ENTITIES];
	auto const object_count = worldGetAllObjects(objects, MAX_ENTITIES);

	for (std::int32_t const i : std::ranges::iota_view { 0, object_count })
	{
		auto const object_handle     = objects[i];

		auto const does_entity_exist = DOES_ENTITY_EXIST(object_handle);
		if (!does_entity_exist)
			continue;
		processor(object_handle);
	}
}

static void ApplyRandomForceToProps(const std::int32_t object)
{

	auto const exists = DOES_ENTITY_EXIST(object);
	if (!exists)
		return;

	auto const inAir = IS_ENTITY_IN_AIR(object);
	if (inAir)
		return;

	auto const randomXForce = static_cast<float>(GET_RANDOM_INT_IN_RANGE(-50, 50));
	auto const randomYForce = static_cast<float>(GET_RANDOM_INT_IN_RANGE(-50, 50));
	auto const randomZForce = static_cast<float>(GET_RANDOM_INT_IN_RANGE(10, 100));

	SET_ENTITY_DYNAMIC(object, true);
	APPLY_FORCE_TO_ENTITY(object, 1, randomXForce, randomYForce, randomZForce, 0, 0, 0, 0, false, false, false,
	                              false, false);
}

static void OnTick()
{
	allobject(ApplyRandomForceToProps);
}

static void OnStart()
{
}

static void OnStop()
{
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
	{
		.Name = "Bouncy Props",
		.Id = "misc_bouncy_props",
		.IsTimed = true,
	}
);