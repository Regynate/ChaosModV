#include <stdafx.h>

#include "Effects/Condition/EffectCondition.h"

static bool OnCondition()
{
	return !IsEnhanced();
}

REGISTER_EFFECT_CONDITION(EffectConditionType::EnhancedShader, OnCondition, "Legacy shader effects do not work on Enhanced");