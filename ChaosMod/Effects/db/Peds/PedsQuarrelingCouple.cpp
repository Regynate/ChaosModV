/*
    Effect by juhana
*/
#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Util/Peds.h"

namespace
{
	using Couple = std::pair<std::uint32_t, std::uint32_t>;

	std::array<Couple, 4> Couples { Couple { "cs_debra"_hash, "cs_floyd"_hash },
		                            Couple { "cs_martinmadrazo"_hash, "cs_patricia"_hash },
		                            Couple { "cs_johnnyklebitz"_hash, "cs_ashley"_hash },
		                            Couple { "player_zero"_hash, "cs_amandatownley"_hash } };
}

static void OnStart()
{
	auto const max          = static_cast<int>(Couples.size());
	auto const randomCouple = GET_RANDOM_INT_IN_RANGE(0, max);

	auto const [ped1, ped2] = Couples[randomCouple];

	CreateHostilePed(ped1, "weapon_pistol"_hash);
	CreateHostilePed(ped2, "weapon_knife"_hash);
}


// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Spawn Quarreling Couple",
		.Id = "peds_spawn_quarreling_couple",
		.EffectGroupType = EffectGroupType::SpawnEnemy
	}
);