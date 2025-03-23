#include "Effects/Register/RegisterEffect.h"
#include "Components/EffectDispatcher.h"
#include <stdafx.h>

static void OnStart()
{
	if (!ComponentExists<EffectDispatcher>())
		return;

	auto const effectId   = GetComponent<EffectDispatcher>()->GetRandomEffectId();
	auto const registeredEffect = GetRegisteredEffect(effectId);

	if (!registeredEffect)
		return;
	LOG(effectId);
	registeredEffect->Start();
	registeredEffect->Tick();
	registeredEffect->Stop();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
	{
		.Name = "Mystery Effect",
		.Id = "misc_mystery_effect"
	}
);

