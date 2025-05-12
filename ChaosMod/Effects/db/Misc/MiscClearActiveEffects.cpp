#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

#include "Components/EffectDispatcher.h"

static void OnStart()
{
	if (ComponentExists<EffectDispatcher>())
		GetComponent<EffectDispatcher>()->ClearActiveEffects("misc_clear_active_effects");
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr,
    {
        .Name = "Clear Active Effects",
        .Id = "misc_clear_active_effects"
    }
);