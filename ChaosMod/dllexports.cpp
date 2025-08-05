#include "dllexports.h"

#include "Components/EffectDispatcher.h"
#include "Effects/EnabledEffects.h"

int GetActiveEffectIDs(const char **outIDs, int maxCount)
{
	int count = 0;

	if (ComponentExists<EffectDispatcher>())
	{
		for (const auto &effect : GetComponent<EffectDispatcher>()->SharedState.ActiveEffects)
		{
            outIDs[count] = effect.Id.Id().c_str();
			++count;
			if (count == maxCount)
				break;
		}
	}

	return count;
}

EffectCategory GetEffectCategory(const char* effectID)
{
	if (!g_EnabledEffects.contains(std::string(effectID)))
		return {};

	return g_EnabledEffects.at(std::string(effectID)).Category;
}