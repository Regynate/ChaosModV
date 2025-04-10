#include <stdafx.h>

#include "Components/MetaModifiers.h"
#include "Effects/Register/RegisterEffect.h"

static void OnTick()
{
	if (ComponentExists<MetaModifiers>())
		GetComponent<MetaModifiers>()->AdditionalEffectsToDispatch = 1;
}

static void OnStop()
{
	if (ComponentExists<MetaModifiers>())
		GetComponent<MetaModifiers>()->AdditionalEffectsToDispatch = 0;
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick,
	{
		.Name = "Bonus Random Effect",
		.Id = "meta_bonus_random_effect",
		.IsTimed = true,
		.ExecutionType = EffectExecutionType::Meta
	}
);