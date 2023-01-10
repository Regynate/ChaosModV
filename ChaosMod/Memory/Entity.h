#pragma once

#include "../Util/Logging.h"
#include "Handle.h"
#include "Memory.h"
#include "../vendor/shv/exports.h"

using DWORD64 = unsigned long long;
using Entity  = int;

namespace Memory
{
	/* FiveM's shv doesn't provide getScriptHandleBaseAddress, so we find it ourselves */
	inline DWORD64 GetScriptHandleBaseAddress(Entity entity)
	{
		// Quick fix because I have no idea how to find this pattern
		/*
		static auto _getScriptHandleBaseAddress = []() -> DWORD64 (*)(Entity)
		{
			Handle handle =
			    FindPattern("48 83 EC 28 E8 ? ? ? ? 48 85 C0 74 19 8B 0D ? ? ? ? 4C 8B 00 48 8D 54 24 ? 89");
			if (!handle.IsValid())
			{
				LOG("Couldn't find _getScriptHandleBaseAddress");
				return nullptr;
			}

			return handle.At(4).Into().Get<DWORD64(Entity)>();
		}();
		*/

		return getScriptHandleBaseAddress(entity);
	}
}
