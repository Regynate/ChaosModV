#include <stdafx.h>

static void onTickBalled()
{
    static DWORD64 lastTick = 0;
    DWORD64 curTick = GET_GAME_TIMER();

    if (curTick > lastTick + 200)
    {
        lastTick = curTick;
        static Hash weaponHash = GET_HASH_KEY("weapon_specialcarbine");
        Ped player = PLAYER_PED_ID();
        float predictionDistanceZ = 6;
        float absoluteDistanceX = 0;
        float absoluteDistanceY = 0;
        float predictionLimit = 6;
        float predictionSpeed = 0;
        float predictionDivider = 3;
        float absoluteVelY = 0;
        float absoluteVelX = 0;
        if (!IS_PED_IN_ANY_VEHICLE(player, false))
        {
            predictionDistanceZ -= 3;
            Vector3 entityVel = GET_ENTITY_VELOCITY(player);
            Vector3 playerCoords = GET_ENTITY_COORDS(player, false);
            if (entityVel.y < 0)
            {
                absoluteVelY = entityVel.y * -1;
            }
            else
            {
                absoluteVelY = entityVel.y;
            }
            if (entityVel.x < 0)
            {
                absoluteVelX = entityVel.x * -1;
            }
            else
            {
                absoluteVelX = entityVel.x;
            }
            float magnitude = std::sqrt((entityVel.x * entityVel.x) + (entityVel.y * entityVel.y));
            if (magnitude < predictionLimit * predictionDivider && magnitude > -predictionLimit * predictionDivider)
            {
                predictionSpeed = magnitude / predictionDivider;
            }
            else
            {
                predictionSpeed = predictionLimit;
            }
            if (predictionSpeed < 0)
            {
                predictionSpeed *= -1;
            }
            float absoluteMagnitude = std::sqrt((absoluteVelX * absoluteVelX) + (absoluteVelY * absoluteVelY));
            float predictionVel = absoluteMagnitude + predictionDistanceZ - entityVel.z - predictionSpeed;
            Vector3 spawnCoords = playerCoords + entityVel / magnitude * predictionSpeed;
            spawnCoords.z = playerCoords.z + predictionDistanceZ;
            Object obj = CreatePoolProp(1441141378, spawnCoords.x, spawnCoords.y, spawnCoords.z, true);
            Vector3 min, max;
            GET_MODEL_DIMENSIONS(1441141378, &min, &max);
            SET_OBJECT_PHYSICS_PARAMS(obj, 50, 1.f, 1.f, 0.f, 0.f, .5f, 0.f, 0.f, 0.f, 0.f, 0.f);
            int backupCounter = 10;
            while (backupCounter > 0 && GET_ENTITY_SPEED(obj) < 0.5f && DOES_ENTITY_EXIST(obj))
            {
                SHOOT_SINGLE_BULLET_BETWEEN_COORDS(spawnCoords.x, spawnCoords.y, spawnCoords.z + max.z - min.z, spawnCoords.x, spawnCoords.y, spawnCoords.z, 0, true, weaponHash, 0, false, true, 0.01);
                SET_ENTITY_VELOCITY(obj, 0.f, 0.f, -predictionVel);
                --backupCounter;
            }
        }
        else
        {
            Vehicle playerVeh = GET_VEHICLE_PED_IS_IN(player, false);
            Vector3 entityVel = GET_ENTITY_VELOCITY(playerVeh);
            Vector3 playerCoords = GET_ENTITY_COORDS(playerVeh, false);
            if (entityVel.y < 0)
            {
                absoluteVelY = entityVel.y * -1;
            }
            else
            {
                absoluteVelY = entityVel.y;
            }
            if (entityVel.x < 0)
            {
                absoluteVelX = entityVel.x * -1;
            }
            else
            {
                absoluteVelX = entityVel.x;
            }
            float magnitude = std::sqrt((entityVel.x * entityVel.x) + (entityVel.y * entityVel.y));
            if (magnitude < predictionLimit * predictionDivider && magnitude > -predictionLimit * predictionDivider)
            {
                predictionSpeed = magnitude / predictionDivider;
            }
            else
            {
                predictionSpeed = predictionLimit;
            }
            if (predictionSpeed < 0)
            {
                predictionSpeed *= -1;
            }
            float absoluteMagnitude = std::sqrt((absoluteVelX * absoluteVelX) + (absoluteVelY * absoluteVelY));
            float predictionVel = absoluteMagnitude + predictionDistanceZ - entityVel.z - predictionSpeed;
            Vector3 spawnCoords = playerCoords + entityVel / magnitude * predictionSpeed;
            spawnCoords.z = playerCoords.z + predictionDistanceZ;
            Object obj = CreatePoolProp(1441141378, spawnCoords.x, spawnCoords.y, spawnCoords.z, true);
            Vector3 min, max;
            GET_MODEL_DIMENSIONS(1441141378, &min, &max);
            SET_OBJECT_PHYSICS_PARAMS(obj, 50, 1.f, 1.f, 0.f, 0.f, .5f, 0.f, 0.f, 0.f, 0.f, 0.f);
            int backupCounter = 10;
            while (backupCounter > 0 && GET_ENTITY_SPEED(obj) < 0.5f && DOES_ENTITY_EXIST(obj))
            {
                SHOOT_SINGLE_BULLET_BETWEEN_COORDS(spawnCoords.x, spawnCoords.y, spawnCoords.z + max.z - min.z, spawnCoords.x, spawnCoords.y, spawnCoords.z, 0, true, weaponHash, 0, false, true, 0.01);
                SET_ENTITY_VELOCITY(obj, 0.f, 0.f, -predictionVel);
                --backupCounter;
            }
        }
    }
}

static RegisterEffect registerEffect(EFFECT_MISC_BLUEBALLED, nullptr, nullptr, onTickBalled, EffectInfo
    {
        .Name = "Get Blueballed",
        .Id = "misc_blueballed",
        .IsTimed = true,
        .EEffectGroupType = EEffectGroupType::SpawnGeneric
    }
);