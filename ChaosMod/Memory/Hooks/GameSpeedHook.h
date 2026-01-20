#pragma once

#include "Util/Events.h"

namespace Hooks
{
	void SetTimeScale(float value);

	inline ChaosEvent OnTimerUpdate;
}