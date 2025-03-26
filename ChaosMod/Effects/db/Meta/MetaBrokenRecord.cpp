#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Components/EffectDispatcher.h"

namespace
{
	std::string randomEffect{};
	std::size_t lastDispatchCount{};
} // unnamed namespace

static void OnStart()
{
	if (ComponentExists<EffectDispatcher>())
		randomEffect = GetComponent<EffectDispatcher>()->GetRandomEffectId();
}

static void OnTick()
{
	if (ComponentExists<EffectDispatcher>())
	{
		auto current = GetComponent<EffectDispatcher>()->EffectDispatchCount;
		if (current != lastDispatchCount)
		{
			GetComponent<EffectDispatcher>()->DispatchEffectForMeta(randomEffect, false);
			lastDispatchCount = current;
		}
	}
		
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, OnTick,
	{
		.Name = "Broken Record",
		.Id = "meta_broken_record",
		.IsTimed = true,
		.ExecutionType = EffectExecutionType::Meta
	}
);
// clang-format on