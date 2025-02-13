#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	for (Ped ped : GetAllPeds())
		REMOVE_ALL_PED_WEAPONS(ped, false);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Remove Weapons From Everyone",
		.Id = "peds_remweps",
		.EffectGroupType = EffectGroupType::Weapons
	}
);