#include <stdafx.h>

#include "Components/EffectDispatcher.h"
#include "Components/MetaModifiers.h"
#include "Effects/Register/RegisterEffect.h"
#include "Components/Voting.h"

namespace
{
	bool originalValue{};
} // unnamed namespace

static void OnStart()
{
	if (ComponentExists<Voting>())
		originalValue = GetComponent<Voting>()->EnableVoting;
}

static void OnStop()
{
	if (ComponentExists<Voting>())
		GetComponent<Voting>()->EnableVoting = originalValue;
}

static void OnTick()
{
	if (ComponentExists<Voting>())
		GetComponent<Voting>()->EnableVoting = false;
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick,
	{
		.Name = "No Vote",
		.Id = "meta_no_vote",
		.IsTimed = true,
		.ExecutionType = EffectExecutionType::Meta
	}
);