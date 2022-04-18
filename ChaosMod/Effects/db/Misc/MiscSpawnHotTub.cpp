/*
    Effect by Moxi based on code by Last0xygen
*/

#include <stdafx.h>

static void OnStart()
{
    Ped player = PLAYER_PED_ID();
    Vector3 pos = GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(player, 0, 5, 0);

    static const Hash tubHash = GET_HASH_KEY("prop_hottub2");
    Object tub = CREATE_OBJECT(tubHash, pos.x, pos.y, pos.z, true, false, true);
    PLACE_OBJECT_ON_GROUND_PROPERLY(tub);
    FREEZE_ENTITY_POSITION(tub, true);
    SET_ENTITY_AS_NO_LONGER_NEEDED(&tub);

    REQUEST_ANIM_DICT("rcmnigel1bnmt_1b");
    WAIT(0);
    //static const Hash girlHash = GET_HASH_KEY(g_Random.GetRandomInt(0, 1) ? "a_f_y_topless_01" : "a_f_y_beach_01");
    static const Hash girlHash = GET_HASH_KEY("a_f_y_topless_01");
    Ped ped = CreatePoolPed(28, girlHash, pos.x, pos.y, pos.z, 0.f);
    SET_PED_RANDOM_COMPONENT_VARIATION(ped, false);

    TASK_PLAY_ANIM(ped, "rcmnigel1bnmt_1b", "dance_loop_tyler", 8.0f, -8.0f, -1.f, 1, 0.f, false, false, false);
    WAIT(0);
    REMOVE_ANIM_DICT("rcmnigel1bnmt_1b");
}

static RegisterEffect registerEffect(EFFECT_SPAWN_HOTTUB, OnStart, nullptr, nullptr, EffectInfo
    {
        .Name = "Spawn Twitch Streamer",
        .Id = "misc_spawn_hottub",
        .EEffectGroupType = EEffectGroupType::SpawnGeneric
    }
);