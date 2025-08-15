#include <stdafx.h>

#include "Memory/Hooks/Hook.h"
#include "Memory/RageSkeleton.h"

// Work around crash related to SET_PED_SHOOTS_AT_COORD
// Thanks to Rainbomizer!
void (*OG_crSkeleton_GetGlobalMtx)(__int64, int, void *);
void HK_crSkeleton_GetGlobalMtx(__int64 skeleton, int id, void *matrix)
{
	if (!skeleton)
		return;

	if (id == -1)
		id = 0;

	OG_crSkeleton_GetGlobalMtx(skeleton, id, matrix);
}

static bool OnHook()
{
	Handle handle;

	handle = Memory::FindPattern("E8 ? ? ? ? 4D 03 F5 48", "E8 ? ? ? ? 41 F6 47 54 08");
	if (!handle.IsValid())
		LOG("crSkeleton::GetGlobalMtx not found!");
	else
	{
		Memory::AddHook(handle.Into().Get<void>(), HK_crSkeleton_GetGlobalMtx, &OG_crSkeleton_GetGlobalMtx);
		Memory::crSkeleton_GetGlobalMtx = HK_crSkeleton_GetGlobalMtx;
	}

	handle =
	    Memory::FindPattern("E8 ?? ?? ?? ?? 48 FF C6 48 83 FE 08", "E8 ?? ?? ?? ?? 48 8B 44 24 28 42 F6 44 20 41 10");
	if (!handle.IsValid())
		LOG("crSkeleton::SetGlobalMtx not found!");
	else
		Memory::crSkeleton_SetGlobalMtx = reinterpret_cast<void (*)(__int64, int, void *)>(handle.Into().Get<void>());

	handle = Memory::FindPattern("E8 ?? ?? ?? ?? 41 FF C6 45 3B F4 7C BE", "E8 ?? ?? ?? ?? EB B0 48 83 C4 20 5B 5D");
	if (!handle.IsValid())
		LOG("rage::crSkeleton::PartialUpdate not found!");
	else
		Memory::rage__crSkeleton__PartialUpdate = handle.Into().Get<void(__int64, unsigned int, bool)>();

	return true;
}

static RegisterHook registerHook(OnHook, nullptr, "MiscHooks", true);