#include <stdafx.h>

#include "Components/Voting.h"
#include "Effects/Condition/EffectCondition.h"

static bool OnCondition()
{
	return ComponentExists<Voting>() && (GetComponent<Voting>()->IsEnabled() || GetComponent<Voting>()->IsSilentEnabled());
}

REGISTER_EFFECT_CONDITION(EffectConditionType::SilentVotingEnabled, OnCondition, "Voting is not enabled");