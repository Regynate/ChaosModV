#include <stdafx.h>

static void OnTick()
{
	std::list<Entity> entities;
	for (auto ped : GetAllPeds())
	{
		if (!IS_PED_IN_ANY_VEHICLE(ped, false))
		{
			entities.push_back(ped);
		}
	}
	for (auto veh : GetAllVehs())
	{
		entities.push_back(veh);
	}

	for (auto entity : entities)
	{
		if (!DOES_ENTITY_EXIST(entity))
		{
			continue;
		}

		Vector3 vel = GET_ENTITY_VELOCITY(entity);

		if (GET_ENTITY_HEIGHT_ABOVE_GROUND(entity) > 2.f && vel.z > -25.f)
		{
			SET_ENTITY_VELOCITY(entity, vel.x, vel.y, std::min(vel.z * 0.66f, vel.z - 1.33f));
		}
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, EffectInfo
	{
		.Name = "Grounded",
		.Id = "misc_grounded",
		.IsTimed = true,
    }
);