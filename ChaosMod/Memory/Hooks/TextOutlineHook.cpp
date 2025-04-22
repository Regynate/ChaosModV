#include <stdafx.h>

#include "TextOutlineHook.h"

#include "Memory/Hooks/Hook.h"

#include "Memory.h"
#include "game.h"

#include "Memory/Allocator/MemoryBuffer.h"

typedef unsigned long long DWORD64;
typedef unsigned long DWORD;
typedef unsigned char BYTE;

static float *strengthPtr;
static float *newRadiusPtr;

static bool OnHook()
{
	if (!IsEnhanced())
		return false;

	auto handle = Memory::FindPattern("48 89 f9 e8 ? ? ? ? c7 44 24 ? ? ? ? ? f2 0f 10 05");

	if (!handle.IsValid())
		return false;

	strengthPtr                   = handle.At(12).Get<float>();

	// some fuckery that's probably really not needed, but ah well
	newRadiusPtr                  = reinterpret_cast<float *>(AllocateBuffer(handle.Get<void>()));

	if (!newRadiusPtr)
		return false;

	Handle handle1                = handle.At(20);

	std::uint32_t newJmpValue     = reinterpret_cast<std::uintptr_t>(newRadiusPtr) - handle1.Addr() - 4;

	*handle1.Get<std::uint32_t>() = newJmpValue;

	return true;
}

static void OnCleanup()
{
	FreeBuffer(newRadiusPtr);
	strengthPtr = newRadiusPtr = nullptr;
}

namespace Hooks
{
	void SetTextOutlineParams(float radius, float strength)
	{
		if (newRadiusPtr)
		{
			newRadiusPtr[0] = radius;
			newRadiusPtr[1] = radius;
		}

		if (strengthPtr)
			*strengthPtr = strength;
	}
}

static RegisterHook registerHook(OnHook, OnCleanup, "_TextOutlineModificator");
