#include "stdafx.h"

#include "Memory.h"

#include "Settings.h"

namespace Memory
{
	struct Settings
	{
		byte pad[0x256];
	};

	static __int64 (*CPauseMenu__GetMenuGraphicsSettings)(Settings *result, bool bForSaving);
	static void (*CPauseMenu__SetValueBasedOnPreference)(int iPrefChanged, int source, int iPreviousValue);
	static void (*CSettingsManager__RequestNewSettings)(__int64 *_this, Settings *settings);
	static __int64 *CSettingsManager__Instance;

	static int *settingsArray;

	static void Init()
	{
		Handle handle = Memory::FindPattern("44 39 2D ?? ?? ?? ?? 0F 85 D6 00 00 00",
		                                    "83 3D ?? ?? ?? ?? ?? 0F 85 28 01 00 00 48 89 F9");
		if (handle.IsValid())
		{
			handle        = handle.At(IsEnhanced() ? 1 : 2).Into();
			settingsArray = handle.At(IsEnhanced() ? -27 * 4 + 1 : -27 * 4).Get<int>();
		}
		else
		{
			LOG("Couldn't find settings array!");
		}

		handle = Memory::FindPattern("E8 ?? ?? ?? ?? 48 89 5C 24 20 8B 88 E8 00 00 00",
		                             "e8 ? ? ? ? 48 8b 44 24 ? 48 89 84 24 ? ? ? ? 48 8d 8c 24");
		if (handle.IsValid())
		{
			handle                              = handle.Into();
			CPauseMenu__GetMenuGraphicsSettings = handle.Get<__int64(Settings *, bool)>();
		}
		else
		{
			LOG("Couldn't find CPauseMenu__GetMenuGraphicsSettings!");
		}

		handle = Memory::FindPattern("E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 84 C0 74 0A E8",
		                             "E8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 84 C0 74 0A E8");
		if (handle.IsValid())
		{
			handle                               = handle.Into();
			CSettingsManager__RequestNewSettings = handle.Get<void(__int64 *, Settings *)>();
		}
		else
		{
			LOG("Couldn't find CSettingsManager__RequestNewSettings!");
		}

		handle = Memory::FindPattern("48 8D 0D ?? ?? ?? ?? 48 8B D0 E8 ?? ?? ?? ?? E8",
		                             "48 8D 0D ?? ?? ?? ?? 44 8B 15 ?? ?? ?? ?? 41 83 FA FF");
		if (handle.IsValid())
		{
			handle                     = handle.At(2).Into();
			CSettingsManager__Instance = handle.Get<__int64>();
		}
		else
		{
			LOG("Couldn't find CSettingsManager__Instance!");
		}

		handle = Memory::FindPattern("E8 ?? ?? ?? ?? FF C3 81 FB B0 00 00 00 7C E9",
		                             "E8 ?? ?? ?? ?? 0F B7 0D ?? ?? ?? ?? 48 85 C9 74 60");
		if (handle.IsValid())
		{
			handle                              = handle.Into();
			CPauseMenu__SetValueBasedOnPreference = handle.Get<void(int, int, int)>();
		}
		else
		{
			LOG("Couldn't find CPauseMenu__SetValueBasedOnPreference!");
		}
	}

	int GetSetting(eMenuPref index)
	{
		if (!settingsArray)
			Init();

        // values are different on legacy and enhanced but i'm too tired to do this properly
        // todo: fix
		if (index < 0 || index >= 1000 || !settingsArray)
			return 0;

		return settingsArray[index];
	}

	void SetSetting(eMenuPref index, int value)
	{
		if (!settingsArray)
			Init();

		if (index < 0 || index >= 1000 || !settingsArray)
			return;

        const auto prev = settingsArray[index];

		settingsArray[index] = value;
        
        if (CPauseMenu__SetValueBasedOnPreference)
        {
            CPauseMenu__SetValueBasedOnPreference(index, 2, -1);
        }
	}

	void ApplySettings()
	{

		if (CPauseMenu__GetMenuGraphicsSettings && CSettingsManager__RequestNewSettings && CSettingsManager__Instance)
		{
			Settings settings;

			CPauseMenu__GetMenuGraphicsSettings(&settings, false);
			CSettingsManager__RequestNewSettings(CSettingsManager__Instance, &settings);
		}
	}
}