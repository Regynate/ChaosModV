#include "Effects/Register/RegisterEffect.h"

#include <stdafx.h>

static float DegreeToRadian(float deg)
{
	auto constexpr PI = 3.1415f;
    const double rad = (PI / 180) * deg;
    return (float)rad;
}

static Vector3 RotationToDirection(Vector3 rot)
{
    float x = DegreeToRadian(rot.x);
    float z = DegreeToRadian(rot.z);

    float num = abs(cos(x));

    return Vector3
    {
        -std::sin(z) * num,
        std::cos(z) * num,
        std::sin(x)
    };
}

static Vector3 GetCoordsInFront(Vector3 pos, Vector3 rot, float dist)
{
    Vector3 ret{};

    float a = pos.x + (RotationToDirection(rot).x * dist);
    float b = pos.y + (RotationToDirection(rot).y * dist);
    float c = pos.z + (RotationToDirection(rot).z * dist);

    ret.x = a;
    ret.y = b;
    ret.z = c;

    return ret;
}

static void OnStart() {
    auto const playerId = PLAYER_ID();
	auto const isPlayerShooting = IS_PLAYER_FREE_AIMING(playerId);

    auto const boomGateModel = GET_HASH_KEY("prop_sec_barrier_ld_01a");

    auto const player = PLAYER_PED_ID();
	auto const coords = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(player, -4.f, 0.f, 0.f);

    auto const rotation = GET_ENTITY_ROTATION(player, 0);
    auto const inFront = GetCoordsInFront(coords, rotation, 5);
    auto const heading = GET_ENTITY_HEADING(player);
	
    LoadModel(boomGateModel);

    float groundZ{};
	GET_GROUND_Z_FOR_3D_COORD(inFront.x, inFront.y, inFront.z, &groundZ, true, false);

    auto const boomGate = CreatePoolProp(boomGateModel, inFront.x, inFront.y, groundZ + 0.5f, true);

    SET_ENTITY_HEADING(boomGate, heading);

    FREEZE_ENTITY_POSITION(boomGate, true);

    WAIT(2000);

    FREEZE_ENTITY_POSITION(boomGate, false);
}

REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Boom Gate",
        .Id = "misc_boom_gate", 
        .IsTimed = false
    }
);
