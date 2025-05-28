#pragma once

#include "Entity.h"
#include "Handle.h"
#include "Memory.h"

#include "WeaponPool.h"

#include "Util/Peds.h"

#include <vector>

namespace Memory
{
	inline const rage::array<DWORD64> *GetAllWeaponPointers();
	inline const std::vector<DWORD64> &GetAllWeaponComponentPointers();

	inline Hash GetWeaponHash(DWORD64 weaponPtr)
	{
		return *reinterpret_cast<Hash *>(weaponPtr + 0x10);
	}

	inline Hash GetWeaponModel(DWORD64 weaponPtr)
	{
		return *reinterpret_cast<Hash *>(weaponPtr + 0x14);
	}

	inline bool IsEntityAWeapon(Entity entity)
	{
		const auto model          = GetEntityModel(entity);

		const auto weaponPtrArray = GetAllWeaponPointers();

		for (int i = 0; i < weaponPtrArray->count; i++)
			if (GetWeaponModel(weaponPtrArray->elements[i]) == model)
				return true;

		return false;
	}

	inline bool IsEntityAWeaponComponent(Entity entity)
	{
		const auto model             = GetEntityModel(entity);

		const auto componentPtrArray = GetAllWeaponComponentPointers();

		for (int i = 0; i < componentPtrArray.size(); i++)
			if (GetWeaponModel(componentPtrArray[i]) == model)
				return true;

		return false;
	}

	inline DWORD64 GetWeaponInfo(Hash ulHash)
	{
		const auto weaponPtrArray = GetAllWeaponPointers();
		if (!weaponPtrArray)
			return 0;

		for (size_t i = 0; i < weaponPtrArray->count; i++)
		{
			const auto weaponPtr = weaponPtrArray->elements[i];
			if (GetWeaponHash(weaponPtr) == ulHash)
				return weaponPtr;
		}

		return 0;
	}

	inline bool IsWeaponRangable(Hash ulWepHash)
	{
		std::vector<Hash> weps = // Basically anything throwable or projectile.
		    { 2726580491, 2982836145, 1305664598, 2982836145, 1752584910, 2481070269, 741814745,
			  4256991824, 2694266206, 615608432,  883325847,  1672152130, 2138347493, 125959754,
			  3676729658, 1834241177, 600439132,  1233104067, 2874559379, 126349499,  3125143736 };

		if (std::count(weps.begin(), weps.end(), ulWepHash))
			return false;
		return true;
	}

	inline void SetWeaponDamage(Hash ulHash, float fDamage)
	{
		auto infoAddr                               = GetWeaponInfo(ulHash);
		*reinterpret_cast<float *>(infoAddr + 0xB0) = fDamage;
	}

	inline float GetWeaponDamage(Hash ulHash)
	{
		auto infoAddr = GetWeaponInfo(ulHash);
		return *reinterpret_cast<float *>(infoAddr + 0xB0);
	}

	inline void SetWeaponRange(Hash ulHash, float fRange)
	{
		auto infoAddr                                = GetWeaponInfo(ulHash);
		*reinterpret_cast<float *>(infoAddr + 0x28C) = fRange;
	}

	inline float GetWeaponRange(Hash ulHash)
	{
		auto infoAddr = GetWeaponInfo(ulHash);
		return *reinterpret_cast<float *>(infoAddr + 0x28C);
	}

	inline void SetWeaponBulletsPerBatch(Hash ulHash, int iBullets)
	{
		auto infoAddr                                   = GetWeaponInfo(ulHash);
		*reinterpret_cast<uint32_t *>(infoAddr + 0x120) = iBullets;
	}

	inline uint32_t GetWeaponBulletsPerBatch(Hash ulHash)
	{
		auto infoAddr = GetWeaponInfo(ulHash);
		return *reinterpret_cast<uint32_t *>(infoAddr + 0x120);
	}

	inline void SetWeaponBatchSpread(Hash ulHash, float fSpread)
	{
		auto infoAddr                                 = GetWeaponInfo(ulHash);
		*reinterpret_cast<float *>(infoAddr + 0x0124) = fSpread;
	}

	inline float GetWeaponBatchSpread(Hash ulHash)
	{
		auto infoAddr = GetWeaponInfo(ulHash);
		return *reinterpret_cast<float *>(infoAddr + 0x0124);
	}

	inline void SetWeaponAccuracySpread(Hash ulHash, float fSpread)
	{
		auto infoAddr                               = GetWeaponInfo(ulHash);
		*reinterpret_cast<float *>(infoAddr + 0x74) = fSpread;
	}

	inline float GetWeaponAccuracySpread(Hash ulHash)
	{
		auto infoAddr = GetWeaponInfo(ulHash);
		return *reinterpret_cast<float *>(infoAddr + 0x74);
	}

	inline void SetWeaponForce(Hash ulHash, float fForce)
	{
		auto infoAddr                                 = GetWeaponInfo(ulHash);
		*reinterpret_cast<float *>(infoAddr + 0x00D8) = fForce;
	}

	inline float GetWeaponForce(Hash ulHash)
	{
		auto infoAddr = GetWeaponInfo(ulHash);
		return *reinterpret_cast<float *>(infoAddr + 0x00D8);
	}

	inline bool IsWeaponShotgun(Hash ulHash)
	{
		auto infoAddr = GetWeaponInfo(ulHash);
		return *reinterpret_cast<int32_t *>(infoAddr + 0x0058) == 5;
	}

	static int GetWeaponGroup(Hash ulHash)
	{
		auto infoAddr = GetWeaponInfo(ulHash);
		return *reinterpret_cast<int *>(infoAddr + 0x5C);
	}

	static void SetWeaponGroup(Hash ulHash, Hash value)
	{
		auto infoAddr                             = GetWeaponInfo(ulHash);
		*reinterpret_cast<int *>(infoAddr + 0x5C) = value;
	}
}