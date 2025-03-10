#include <stdafx.h>
#include "Effects/Register/RegisterEffect.h"

static void GetAllPeds(void (*processor)(std::int32_t))
{
    static constexpr std::int32_t maxEntities = 100;
    std::int32_t peds[maxEntities];
    auto const pedCount = worldGetAllPeds(peds, maxEntities);

    auto const player = PLAYER_PED_ID();

    for (auto const i : std::ranges::iota_view{ 0, pedCount }) {
        auto const pedHandle = peds[i];

        if (!DOES_ENTITY_EXIST(pedHandle) || pedHandle == player) {
            continue;
        }

        if (IS_ENTITY_A_MISSION_ENTITY(pedHandle) || IS_PED_IN_ANY_VEHICLE(pedHandle, true)) {
            continue;
        }

        processor(pedHandle);
    }
}


static auto constexpr grenadeCount = 5;
static std::unordered_set<std::int32_t> processedPeds;


static void ShootGrenadesOnDeath(std::int32_t ped)
{
    if (!DOES_ENTITY_EXIST(ped) || !IS_PED_DEAD_OR_DYING(ped, false)) {
        return;
    }

    if (processedPeds.contains(ped)) {
        return;
    }

    processedPeds.insert(ped);
    auto const pedCoords = GET_ENTITY_COORDS(ped, true);
    auto const grenadeHash = GET_HASH_KEY("weapon_grenade");

    for (auto i = 0; i < grenadeCount; i++) {
        auto const offsetX = GET_RANDOM_FLOAT_IN_RANGE(-0.5f, 0.5f);
        auto const offsetY = GET_RANDOM_FLOAT_IN_RANGE(-0.5f, 0.5f);
        auto const grenadeX = pedCoords.x + offsetX;
        auto const grenadeY = pedCoords.y + offsetY;
        auto const grenadeZ = pedCoords.z + 0.5f;

        auto const targetX = grenadeX + GET_RANDOM_FLOAT_IN_RANGE(-2.0f, 2.0f);
        auto const targetY = grenadeY + GET_RANDOM_FLOAT_IN_RANGE(-2.0f, 2.0f);
        auto const targetZ = grenadeZ + GET_RANDOM_FLOAT_IN_RANGE(1.0f, 3.0f);

        SHOOT_SINGLE_BULLET_BETWEEN_COORDS(
            grenadeX, grenadeY, grenadeZ,
            targetX, targetY, targetZ,
            100, true, grenadeHash, ped, true, false, 1.0f
        );
    }
}

static void OnStart()
{
    auto const player = PLAYER_PED_ID();
    auto const grenadeHash = GET_HASH_KEY("weapon_grenade");
    GIVE_DELAYED_WEAPON_TO_PED(player, grenadeHash, 1, false);
    processedPeds.clear();
}

static void OnStop()
{
    processedPeds.clear();
}

static void OnTick()
{
    GetAllPeds(ShootGrenadesOnDeath);
}

REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "A Bad Year",
        .Id = "peds_a_bad_year",
        .IsTimed = true
    }
);
