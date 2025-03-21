#include "Effects/Register/RegisterEffect.h"
#include "Components/EffectDispatcher.h"
#include <stdafx.h>


static void OnStart() {
	if (!ComponentExists<EffectDispatcher>())
		return;

    auto const effect = GetComponent<EffectDispatcher>()->GetLastEffectId();
	if (effect.empty())
		return;

    GetComponent<EffectDispatcher>()->DispatchEffect(effect);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Repeat Last Effect", 
        .Id = "misc_repeat_last_effect", 
    }
);
