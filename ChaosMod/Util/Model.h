#pragma once

#include "Natives.h"

#include "Util/Script.h"

inline void LoadModel(Hash model)
{
	if (IS_MODEL_VALID(model))
	{
		REQUEST_MODEL(model);
		while (!HAS_MODEL_LOADED(model))
		{
			WAIT(0);
		}
	}
}

inline void UnloadModel(Hash ulHash)
{
	if (IS_MODEL_VALID(ulHash))
	{
		SET_MODEL_AS_NO_LONGER_NEEDED(ulHash);
	}
}