#include <stdafx.h>
#include "Effects/Register/RegisterEffect.h"

static void OnTick()
{
    auto const player = PLAYER_PED_ID();
    auto const playerId = PLAYER_ID();

    auto const isPlayerShooting = IS_PED_SHOOTING(player);
    if (!isPlayerShooting) {
        return;
    }

    Entity entity{};
    GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(playerId, &entity);

    auto const currentHealth = GET_ENTITY_HEALTH(player);

    if (IS_ENTITY_A_PED(entity)) {
        SET_ENTITY_HEALTH(player, currentHealth - 1, 0);
        entity = 0;
    }
    else {
        SET_ENTITY_HEALTH(player, currentHealth - 10, 0);
    }
}

REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Blood Sacrifice",
        .Id = "player_blood_sacrifice", 
        .IsTimed = true
    }
);
