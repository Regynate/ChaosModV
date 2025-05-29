#include <stdafx.h>

#include "TextOutlineHook.h"

#include "Memory/Hooks/Hook.h"

#include "Memory.h"
#include "game.h"

#include "Memory/Allocator/MemoryBuffer.h"

#include "Memory/Rain.h"

static std::optional<float> desiredRotation;
static std::optional<ChaosVector3> desiredPosition;

static float (*OG_GetMinimapRotation)();
static float HK_GetMinimapRotation()
{
	return OG_GetMinimapRotation() + desiredRotation.value_or(0.f);
}

static void (*OG_GetMinimapPosition)(ChaosVector3 *out);
static void HK_GetMinimapPosition(ChaosVector3 *out)
{
	OG_GetMinimapPosition(out);

	if (desiredPosition.has_value())
		*out = *out + desiredPosition.value();
}

static bool OnHook()
{
	auto handle = Memory::FindPattern("E8 ?? ?? ?? ?? F3 0F 59 05 ?? ?? ?? ?? F3 0F 2C E8 8B C5",
	                                  "E8 ?? ?? ?? ?? F3 0F 59 05 ?? ?? ?? ?? F3 0F 2C E8 41 81 FF");

	if (!handle.IsValid())
		return false;

	Memory::AddHook(handle.Into().Get<void>(), HK_GetMinimapRotation, &OG_GetMinimapRotation);

	handle = Memory::FindPattern("E8 ?? ?? ?? ?? 33 C9 E8 ?? ?? ?? ?? 0F 57 F6",
	                             "E8 ?? ?? ?? ?? 31 C9 E8 ?? ?? ?? ?? 84 C0 74 0B 0F B6 05 ?? ?? ?? ?? 24 01 75 36");

	if (!handle.IsValid())
		return false;

	Memory::AddHook(handle.Into().Get<void>(), HK_GetMinimapPosition, &OG_GetMinimapPosition);

	return true;
}

static void OnCleanup()
{
	desiredRotation = std::nullopt;
}

namespace Hooks
{
	void SetMinimapRotation(float rotation)
	{
		desiredRotation = rotation;
	}

	void ResetMinimapRotation()
	{
		desiredRotation = std::nullopt;
	}

	void SetMinimapPosition(Vector3 position)
	{
		desiredPosition = ChaosVector3(position);
	}

	void ResetMinimapPosition()
	{
		desiredPosition = std::nullopt;
	}
}

static RegisterHook registerHook(OnHook, OnCleanup, "_MinimapParams");
