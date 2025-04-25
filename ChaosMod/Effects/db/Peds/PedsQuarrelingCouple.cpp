#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Util/Peds.h"

namespace
{
	struct CoupleData
	{
		Hash ped1;
		Hash ped2;
		Hash weapon1;
		Hash weapon2;
	};

	std::array<CoupleData, 4> Couples { CoupleData { "cs_debra"_hash, "cs_floyd"_hash, "weapon_pistol"_hash, "weapon_pumpshotgun"_hash },
		                                CoupleData { "cs_martinmadrazo"_hash, "cs_patricia"_hash, "weapon_carbinerifle"_hash, "weapon_bat"_hash },
										CoupleData { "cs_johnnyklebitz"_hash, "cs_ashley"_hash, "weapon_assaultrifle"_hash, "weapon_revolver"_hash },
										CoupleData { "player_zero"_hash, "cs_amandatownley"_hash, "weapon_combatmg"_hash, "weapon_hammer"_hash }
	};
}

static void OnStart()
{
	auto const max                            = static_cast<int>(Couples.size());
	auto const randomCouple                   = GET_RANDOM_INT_IN_RANGE(0, max);

	auto const [ped1, ped2, weapon1, weapon2] = Couples[randomCouple];

	CreateHostilePed(ped1, weapon1);
	CreateHostilePed(ped2, weapon2);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Spawn Quarreling Couple",
        .Id = "peds_spawn_quarreling_couple",
        .EffectGroupType = EffectGroupType::SpawnEnemy
    }
);
