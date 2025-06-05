#pragma once

#include "Lib/array.h"

#include "Memory.h"

#include "Weapon.h"

#include "../Util/Natives.h"

#include <vector>

using DWORD64 = unsigned long long;
using DWORD   = unsigned long;
using WORD    = unsigned short;

namespace Memory
{
	inline Hash GetWeaponHash(uintptr_t weaponPtr);

	inline const rage::array<DWORD64> *GetAllWeaponPointers()
	{
		static Handle handle =
		    Memory::FindPattern("74 42 0F B7 15 ? ? ? 01", "48 89 35 ?? ?? ?? ?? 0F B7 2D ?? ?? ?? ?? E9 06 FE FF FF");
		if (!handle.IsValid())
			return nullptr;

		if (IsLegacy())
			return handle.At(18).Into().Get<rage::array<DWORD64>>();
		else
			return handle.At(2).Into().Get<rage::array<DWORD64>>();
	}

	inline const std::vector<Hash> &GetAllWeapons()
	{
		static std::vector<Hash> weapons;

		if (weapons.empty())
		{
			// Get address of CWeaponInfo's vftable and store it
			static Handle handle = Memory::FindPattern("48 8D 05 ? ? ? ? 4C 89 71 08 4C 89 71 10",
			                                           "48 8D 05 ?? ?? ?? ?? 48 89 01 C7 41 5C 00 00 00 00");
			if (!handle.IsValid())
				return weapons;

			const auto weaponPtrArray = GetAllWeaponPointers();

			if (!weaponPtrArray)
				return weapons;

			auto CWeaponInfo_vftable = handle.At(2).Into().Addr();

			DWORD64 weaponPtr;

			for (int i = weaponPtrArray->count - 1; i >= 0; i--)
			{
				weaponPtr = weaponPtrArray->elements[i];

				// Only include actual ped weapons by checking if vftable pointed to is CWeaponInfo's
				if (*reinterpret_cast<DWORD64 *>(weaponPtr) != CWeaponInfo_vftable)
					continue;

				// Check if weapon has valid model & slot
				if (*reinterpret_cast<DWORD *>(weaponPtr + 20) && *reinterpret_cast<DWORD *>(weaponPtr + 28))
				{
					Hash weaponHash = GetWeaponHash(weaponPtr);

					// Blacklist the remaining invalid weapons I found
					switch (weaponHash)
					{
					case 4256881901:
					case 2294779575:
					case 1834887169:
					case 1752584910:
					case 849905853:
					case 375527679:
					case 28811031:
						continue;
					}

					weapons.push_back(weaponHash);
				}
			}
		}

		return weapons;
	}

	inline const std::vector<DWORD64> &GetAllWeaponComponentPointers()
	{
		static std::vector<DWORD64> components;

		if (components.empty())
		{
			auto handle = Memory::FindPattern("4C 8D 0D ?? ?? ?? ?? 41 D1 FB", "4C 8D 3D ?? ?? ?? ?? EB 09");

			if (!handle.IsValid())
				return components;

			const auto componentPtrArray = handle.At(2).Into().Get<DWORD64>();

			handle = Memory::FindPattern("44 8B 15 ?? ?? ?? ?? 41 8B DC", "44 8B 1D ?? ?? ?? ?? 45 8D 53 FF");

			if (!handle.IsValid())
				return components;

			const auto componentCount = handle.At(2).Into().Value<DWORD>();

			for (size_t i = 0; i < componentCount; i++)
				if (componentPtrArray[i])
					components.push_back(componentPtrArray[i]);
		}

		return components;
	}
}