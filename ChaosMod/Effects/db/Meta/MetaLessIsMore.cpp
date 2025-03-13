#include <stdafx.h>

#include "Components/MetaModifiers.h"
#include "Effects/Register/RegisterEffect.h"

static void OnStop()
{
	if (ComponentExists<MetaModifiers>())
		GetComponent<MetaModifiers>()->EffectDurationModifier = 1.f;
}

static void OnTick()
{
	if (ComponentExists<MetaModifiers>())
		GetComponent<MetaModifiers>()->EffectDurationModifier = 0.1f;
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick,
	{
		.Name = "Less Is More",
		.Id = "meta_less_is_more",
		.IsTimed = true,
		.ExecutionType = EffectExecutionType::Meta
	}
);
