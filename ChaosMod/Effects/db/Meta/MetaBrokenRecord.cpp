#include <stdafx.h>

#include "Components/EffectDispatcher.h"
#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR EffectIdentifier effectId;
CHAOS_VAR CHAOS_EVENT_LISTENER(EffectDispatcher::OnPostDispatchEffect) onEffectDispatchListener;

static std::set<std::string> blacklistedEffects
{
	"meta_broken_record",
	"meta_re_invoke",
	"misc_repeat_last_effect"
};

static void OnStart()
{
	if (ComponentExists<EffectDispatcher>())
	{
		onEffectDispatchListener.Register(GetComponent<EffectDispatcher>()->OnPostDispatchEffect,
		                                  [&](const EffectIdentifier &id, const std::string context)
		                                  {
			                                  if (id != "meta_broken_record" && context != "meta_broken_record")
				                                  GetComponent<EffectDispatcher>()->DispatchEffect(
				                                      effectId, EffectDispatcher::DispatchEffectFlag_None, "",
				                                      "meta_broken_record");
			                                  return true;
		                                  });

		effectId = EffectIdentifier("misc_repeat_last_effect"); // GetComponent<EffectDispatcher>()->GetLastEffectId();
		while (effectId == "" || blacklistedEffects.contains(effectId))
		{
			// will crash if none of the effects are enabled, but in what world is that the real scenario
			effectId = GetComponent<EffectDispatcher>()->GetRandomEffectId();
		}
	}
}

static void OnStop()
{
	if (ComponentExists<EffectDispatcher>())
		onEffectDispatchListener.Unregister(GetComponent<EffectDispatcher>()->OnPostDispatchEffect);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr,
	{
		.Name = "Broken Record",
		.Id = "meta_broken_record",
		.IsTimed = true,
		.ExecutionType = EffectExecutionType::Meta
	}
);
// clang-format on