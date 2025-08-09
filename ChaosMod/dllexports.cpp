#include "dllexports.h"

#include "Components/EffectDispatcher.h"
#include "Effects/EnabledEffects.h"

int GetActiveEffectIDs(char **outIDs, int maxLen, int maxCount)
{
	if (maxCount <= 0)
		return 0;

	int count = 0;

	if (ComponentExists<EffectDispatcher>())
	{
		for (const auto &effect : GetComponent<EffectDispatcher>()->SharedState.ActiveEffects)
		{
			if (count >= maxCount)
				break;

			std::string_view id_str = effect.Id.Id();
			if (maxLen >= 1)
			{
				std::size_t charsToCopy = std::min(id_str.length(), static_cast<size_t>(maxLen - 1));
				
				id_str.copy(outIDs[count], charsToCopy);
				outIDs[count][charsToCopy] = '\0';
			}

			++count;
		}
	}

	return count;
}

int GetEffectCategory(const char *effectID)
{
	if (!g_EnabledEffects.contains(std::string(effectID)))
		return 0;

	return static_cast<int>(g_EnabledEffects.at(std::string(effectID)).Category);
}