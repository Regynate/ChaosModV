/*
Effect by Last0xygen & Moxi
*/

#include <stdafx.h>

static void OnStart()
{
    static Hash enemyHash = GET_HASH_KEY("a_m_m_acult_01");
    static Hash weaponHash = GET_HASH_KEY("weapon_stone_hatchet");
    Ped ped = CreateHostilePed(enemyHash, weaponHash);

    SET_PED_COMPONENT_VARIATION(ped, 3, 0, 0, 0);
    SET_PED_COMPONENT_VARIATION(ped, 9, 0, 0, 0);

}

static RegisterEffect registerEffect(EFFECT_PEDS_SPAWN_ANGRY_NAKED_MAN, OnStart, nullptr, nullptr, EffectInfo
    {
        .Name = "Spawn Angry Naked Man",
        .Id = "peds_spawn_angry_naked_man",
        .EEffectGroupType = EEffectGroupType::SpawnEnemy
    }
);
