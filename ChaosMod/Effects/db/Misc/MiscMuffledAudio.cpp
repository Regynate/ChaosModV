/*
    Effect By Rylxnd
*/
#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Memory/Hooks/AudioClearnessHook.h"

static void OnStart()
{
	Hooks::SetAudioClearness(0);
}

static void OnStop()
{
	Hooks::ResetAudioClearness();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr, 
	{
		.Name = "Muffled Audio",
		.Id = "misc_muffled_audio",
		.IsTimed = true,
	}
);
