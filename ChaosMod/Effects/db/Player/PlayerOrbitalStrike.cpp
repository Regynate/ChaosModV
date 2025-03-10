#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	auto const player = PLAYER_PED_ID();
	auto const coords = GET_ENTITY_COORDS(player, false);

	ADD_EXPLOSION(coords.x + 5, coords.y, coords.z, 29, 1000.f, true, false, 1.f, false);
	ADD_EXPLOSION(coords.x + 10, coords.y, coords.z, 29, 1000.f, true, false, 1.f, false);
	ADD_EXPLOSION(coords.x - 5, coords.y, coords.z, 29, 1000.f, true, false, 1.f, false);
	ADD_EXPLOSION(coords.x - 10, coords.y, coords.z, 29, 1000.f, true, false, 1.f, false);
	ADD_EXPLOSION(coords.x, coords.y + 5, coords.z, 29, 1000.f, true, false, 1.f, false);
	ADD_EXPLOSION(coords.x, coords.y + 10, coords.z, 29, 1000.f, true, false, 1.f, false);
	ADD_EXPLOSION(coords.x, coords.y - 5, coords.z, 29, 1000.f, true, false, 1.f, false);
	ADD_EXPLOSION(coords.x, coords.y - 10, coords.z, 29, 1000.f, true, false, 1.f, false);
}

static void OnTick()
{

}

static void OnStop()
{

}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Orbital Strike",
		.Id = "player_orbital_strike",
		.IsTimed = false
	}
);