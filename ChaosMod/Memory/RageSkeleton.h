#pragma once

#include "Entity.h"
#include "EntityFragments.h"
#include "Handle.h"
#include "Memory.h"
#include "Util/Types.h"

namespace Memory
{
	inline void (*crSkeleton_GetGlobalMtx)(__int64 skeleton, int id, void *matrix);
	inline void (*crSkeleton_SetGlobalMtx)(__int64 skeleton, int id, void *matrix);
	typedef ChaosMatrix4x4 *(GetThirdPersonSkeletonObjectMtxType)(__int64 entity, int boneIndex);
	inline void (*rage__crSkeleton__PartialUpdate)(__int64 skeleton, unsigned int boneIdx, bool inclusive);

	inline __int64 GetSkeleton(Entity entity)
	{
		if (!GetScriptHandleBaseAddress(entity))
			return 0;

		const auto fragInst = (__int64)EntityFragment::GetFragInst(entity);

		__int64 skeleton    = 0;

		if (fragInst)
		{
			const auto v8 = *reinterpret_cast<__int64 *>(fragInst + 104);
			if (v8 && *reinterpret_cast<__int64 *>(fragInst + 120))
				skeleton = *reinterpret_cast<__int64 *>(v8 + 376);
		}
		else
		{
			const auto v9 = *(reinterpret_cast<__int64 *>(GetScriptHandleBaseAddress(entity)) + 80);
			if (v9)
				skeleton = *reinterpret_cast<__int64 *>(v9 + 40);
		}

		return skeleton;
	}

	inline void GetBoneMatrix(Entity entity, int boneIndex, ChaosMatrix4x4 *matrix)
	{
		auto skeleton = GetSkeleton(entity);

		if (!skeleton || boneIndex == -1)
			return;

		if (crSkeleton_GetGlobalMtx)
			crSkeleton_GetGlobalMtx(skeleton, boneIndex, matrix);
	}

	inline void SetBoneMatrix(Entity entity, int boneIndex, ChaosMatrix4x4 *matrix)
	{
		auto skeleton = GetSkeleton(entity);

		if (!skeleton || boneIndex == -1)
			return;

		if (crSkeleton_SetGlobalMtx)
			crSkeleton_SetGlobalMtx(skeleton, boneIndex, matrix);

		if (rage__crSkeleton__PartialUpdate)
			rage__crSkeleton__PartialUpdate(skeleton, boneIndex, false);
	}
}