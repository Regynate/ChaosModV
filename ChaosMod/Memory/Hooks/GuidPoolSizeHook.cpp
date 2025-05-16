#include <stdafx.h>

#include "Memory/Hooks/Hook.h"

static DWORD64 ms_jzAddr = 0;

static int ms_ogPoolSize;
static int* ms_ogPoolSizePtr;

static BYTE ms_ogByte;

static bool OnHook()
{
	if (IsLegacy())
		return false;

	Handle handle = Memory::FindPattern("B8 D7 A8 11 73").At(-13);
	if (!handle.IsValid())
	{
		LOG("GuidPoolSize: Failed to patch pool size!");

		return false;
	}
	else
	{
        ms_ogPoolSizePtr = handle.At(1).Get<int>();
        ms_ogPoolSize   = *ms_ogPoolSizePtr;

        *ms_ogPoolSizePtr = 70000;

		ms_jzAddr = handle.At(7).Addr();

		ms_ogByte = *reinterpret_cast<BYTE *>(ms_jzAddr);

        // replace jz with jmp
        Memory::Write<BYTE>(reinterpret_cast<BYTE *>(ms_jzAddr), 0xEB);

		LOG("GuidPoolSize: Patched pool size");

		return true;
	}
}

static void OnCleanup()
{
	if (ms_jzAddr)
	{
		// replace jmp with jz
        Memory::Write<BYTE>(reinterpret_cast<BYTE *>(ms_jzAddr), ms_ogByte);
	}
}

static RegisterHook registerHookBypass(OnHook, OnCleanup, "GuidPoolSize");