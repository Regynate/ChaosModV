#include <stdafx.h>

#include "Components/Voting.h"
#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	if (ComponentExists<Voting>())
		GetComponent<Voting>()->Enable();
}

static void OnStop()
{
	if (ComponentExists<Voting>())
		GetComponent<Voting>()->Disable();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr,
	{
		.Name = "Enable Voting",
		.Id = "meta_enable_voting",
		.IsTimed = true,
		.ExecutionType = EffectExecutionType::Meta,
		.ConditionType = EffectConditionType::VotingDisabled
	}
);