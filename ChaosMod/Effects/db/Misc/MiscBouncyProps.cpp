#include <stdafx.h>
#include "Effects/Register/RegisterEffect.h"


static void ApplyRandomForceToProps()
{
	for (auto const object : GetAllProps())
	{
		if (HAS_ENTITY_COLLIDED_WITH_ANYTHING(object))
		{
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
	}
}

static void OnTick()
{
	ApplyRandomForceToProps();
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
	{
		.Name = "Bouncy Props",
		.Id = "misc_bouncy_props",
		.IsTimed = true,
	}
);