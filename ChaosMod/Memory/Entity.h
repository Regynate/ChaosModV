#pragma once

#include "../Util/Logging.h"
#include "Handle.h"
#include "Memory.h"

#include <scripthookv/inc/main.h>

using DWORD64 = unsigned long long;
using Entity  = int;

enum class EntityType : unsigned char
{
	ENTITY_TYPE_NOTHING,
	ENTITY_TYPE_BUILDING,
	ENTITY_TYPE_ANIMATED_BUILDING,
	ENTITY_TYPE_VEHICLE,
	ENTITY_TYPE_PED,
	ENTITY_TYPE_OBJECT,
};

namespace Memory
{
	/* FiveM's shv doesn't provide getScriptHandleBaseAddress, so we find it ourselves */
	inline DWORD64 GetScriptHandleBaseAddress(Entity entity)
	{
		static auto _getScriptHandleBaseAddress = []() -> DWORD64 (*)(Entity)
		{
			auto handle =
			    FindPattern("E8 ? ? ? ? 48 8B D8 48 85 C0 74 3D E8", "E8 ?? ?? ?? ?? 48 85 C0 74 74 48 8B 38");
			if (!handle.IsValid())
			{
				LOG("Couldn't find _getScriptHandleBaseAddress, falling back to shv");
				return reinterpret_cast<DWORD64 (*)(Entity)>(getScriptHandleBaseAddress);
			}

			return handle.Into().Get<DWORD64(Entity)>();
		}();

		return _getScriptHandleBaseAddress(entity);
	}

	inline EntityType GetEntityType(DWORD64 address)
	{
		if (!address) {
			return EntityType::ENTITY_TYPE_NOTHING;
		}
		
		return *reinterpret_cast<EntityType*>(address + 0x28);
	}

	inline EntityType GetEntityType(Entity entity)
	{
		return GetEntityType(GetScriptHandleBaseAddress(entity));
	}
}