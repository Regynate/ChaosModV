#include <stdafx.h>
#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
    auto const player = PLAYER_PED_ID();
    SET_ENTITY_PROOFS(player, false, false, true, false, false, false, false, false);
}

static void OnStop()
{
    auto const player = PLAYER_PED_ID();
    SET_ENTITY_PROOFS(player, false, false, false, false, false, false, false, false);
}

static void OnTick()
{
    auto const player = PLAYER_PED_ID();
    auto const coords = GET_ENTITY_COORDS(player, false);

    auto constexpr EXP_TAG_BZGAS = 21;
    ADD_EXPLOSION(coords.x, coords.y, coords.z, EXP_TAG_BZGAS, 1000.f, true, false, 0.f, true);

    WAIT(50);
}

REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Axe Body Spray",
        .Id = "player_axe_body_spray", 
        .IsTimed = true
    }
);
