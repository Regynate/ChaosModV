#pragma once

#include "Lib/array.h"

#include "Memory.h"

#include "../Util/Natives.h"

#include <vector>

using DWORD64 = unsigned long long;
using DWORD   = unsigned long;
using WORD    = unsigned short;

namespace Memory
{
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

			int v3;
			DWORD64 v4;
			DWORD64 vftableAddrPtr;

			for (v3 = weaponPtrArray->count - 1; v3 >= 0; v3 = v4 - 1)
			{
				v4             = static_cast<DWORD>(v3);

				vftableAddrPtr = weaponPtrArray->elements[v4];

				// Only include actual ped weapons by checking if vftable pointed to is CWeaponInfo's
				if (*reinterpret_cast<DWORD64 *>(vftableAddrPtr) != CWeaponInfo_vftable)
					continue;

				// Check if weapon has valid model & slot
				if (*reinterpret_cast<DWORD *>(vftableAddrPtr + 20) && *reinterpret_cast<DWORD *>(vftableAddrPtr + 28))
				{
					Hash weaponHash = *reinterpret_cast<Hash *>(vftableAddrPtr + 16);

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
}