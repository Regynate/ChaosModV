#include <stdafx.h>

#include "GameSpeedHook.h"

#include "Memory/Hooks/Hook.h"

#include "Util/Logging.h"

static float targetScale           = 1.f;

static float prevTargetScale       = 0.f;
static float prevOgTimeScale       = 1.f;
static float prevAdjustedTimeScale = 1.f;

static float *sm_fTimeScale;
static float (*smTimeScales)[4];
static float ogTimeScales[4];

static float *sm_fMinimumFrameTime;
static float *sm_fMaximumFrameTime;

void (*_OG_rage__fwTimer__Update)(void *_this);
void _HK_rage__fwTimer__Update(void *_this)
{
	if (targetScale > 1.f)
	{
		for (size_t i = 0; i < 4; i++)
		{
			if ((*smTimeScales)[i] != 10.f)
				ogTimeScales[i] = (*smTimeScales)[i];
			(*smTimeScales)[i] = 10.f;
		}

		float minScale = 1.f;
		for (size_t i = 0; i < 4; i++)
			if (ogTimeScales[i] < minScale)
				minScale = ogTimeScales[i];

		*sm_fTimeScale = targetScale * minScale;
	}
	else
	{
		for (size_t i = 0; i < 4; i++)
			if ((*smTimeScales)[i] == 10.f)
				(*smTimeScales)[i] = ogTimeScales[i];

		if ((*smTimeScales)[1] != prevAdjustedTimeScale)
			prevOgTimeScale = (*smTimeScales)[1];
		prevAdjustedTimeScale = (*smTimeScales)[1] = prevOgTimeScale * targetScale;
	}

	if (sm_fMinimumFrameTime)
		*sm_fMinimumFrameTime = targetScale < 0.1f ? 0.00000001f : 0.0003333f;
	if (sm_fMaximumFrameTime)
		*sm_fMaximumFrameTime = targetScale > 1.f ? 100.f : 0.06666f;

	_OG_rage__fwTimer__Update(_this);
}

static bool OnHook()
{
	// todo: make work on legacy
	if (IsLegacy())
		return false;

	sm_fMinimumFrameTime = []() -> float *
	{
		Handle handle = Memory::FindPattern("0F 28 D9 48 8B D1 F3", "F3 0F 10 15 ?? ?? ?? ?? 41 0F 28 CE");
		if (!handle.IsValid())
		{
			LOG("Could not patch sm_MinimumFrameTime!");
			return nullptr;
		}

		return handle.At(IsLegacy() ? 0x23 : 0x3).Into().Get<float>();
	}();

	// todo: test on legacy

	sm_fMaximumFrameTime = []() -> float *
	{
		Handle handle = Memory::FindPattern("0F 28 D9 48 8B D1 F3", "F3 44 0F 10 35 ?? ?? ?? ?? F3 0F 10 15");
		if (!handle.IsValid())
		{
			LOG("Could not patch sm_MaximumFrameTime!");
			return nullptr;
		}

		return handle.At(IsLegacy() ? 0x2B : 0x4).Into().Get<float>();
	}();

	sm_fTimeScale = []() -> float *
	{
		Handle handle = Memory::FindPattern("F3 44 0F 11 15 ?? ?? ?? ?? E8");
		if (!handle.IsValid())
		{
			LOG("Could not find sm_fTimeScale!");
			return nullptr;
		}

		return handle.At(0x4).Into().Get<float>();
	}();

	smTimeScales = []() -> float (*)[4]
	{
		Handle handle = Memory::FindPattern("48 8D 05 ?? ?? ?? ?? BA 03 00 00 00 F3 0F 10 10",
		                                    "F3 0F 5D 35 ?? ?? ?? ?? F3 44 0F 10 05 ?? ?? ?? ?? F3 0F 10 05")
		                    .At(IsLegacy() ? 2 : 3)
		                    .Into();
		if (!handle.IsValid())
		{
			LOG("Could not find smTimeScales!");
			return nullptr;
		}

		return handle.Get<float[4]>();
	}();

	auto handle = Memory::FindPattern("48 81 EC B8 00 00 00 44 0F 29 B4 24 A0 00 00 00 66");
	if (!handle.IsValid())
		return false;

	Memory::AddHook(handle.Get<void>(), _HK_rage__fwTimer__Update, &_OG_rage__fwTimer__Update);

	return true;
}

static RegisterHook registerHook(OnHook, nullptr, "rage::fwTimer::Update");

namespace Hooks
{
	void SetTimeScale(float value)
	{
		targetScale = value;
	}
}