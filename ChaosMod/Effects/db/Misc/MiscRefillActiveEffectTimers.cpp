#include <stdafx.h>

#include "Components/EffectDispatcher.h"
#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	if (ComponentExists<EffectDispatcher>())
	{
		const auto runningEffects = GetComponent<EffectDispatcher>()->SharedState.ActiveEffects;

		for (size_t i = 0; i < runningEffects.size(); i++)
		{
			if (runningEffects[i].IsTimed)
			{
				GetComponent<EffectDispatcher>()->SetRemainingTimeForEffect(runningEffects[i].Id,
				                                                            runningEffects[i].MaxTime);
			}
		}
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr,
	{
		.Name = "Refill Active Effect Timers",
		.Id = "misc_active_timers_refill"
	}
);