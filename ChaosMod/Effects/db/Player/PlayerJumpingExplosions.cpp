#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnTick()
{
	auto const player     = PLAYER_PED_ID();
	auto const isPlayerJumping = IS_PED_JUMPING(player);

	    if (isPlayerJumping)
	    {
		    SET_ENTITY_PROOFS(player, false, false, true, false, false, false, false, false);
		    WAIT(100);
		    auto const coords = GET_ENTITY_COORDS(player, false);
		    ADD_EXPLOSION(coords.x, coords.y, coords.z - 1, 5, 1000.f, true, false, 1.f, false);
		    WAIT(750);
	    }
	    else
		    SET_ENTITY_PROOFS(player, false, false, false, false, false, false, false, false);

	   
}

static void OnStop()
{
	
}

static void OnStart()
{
	
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
	{
		.Name = "Jumping Explosions",
		.Id = "player_jumping_explosions",
		.IsTimed = true,
	}
);
