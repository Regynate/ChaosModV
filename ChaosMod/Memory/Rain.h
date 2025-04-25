#pragma once

#include "Handle.h"
#include "Memory.h"
#include "Util/Types.h"

namespace Memory
{
	static inline uintptr_t GetRainAddress()
	{
		static Handle handle =
		    Memory::FindPattern("48 8D 2D ? ? ? ? 0F 2F C6 0F 97 C0 88 44 24 40 84 C0 74 4A 8B 0D ? ? ? ? 83 CB FF 44",
		                        "48 8d 0d ? ? ? ? 31 db 66 66 66 2e 0f 1f 84 00");
		if (!handle.IsValid())
			return 0;

		return handle.At(2).Into().Addr();
	}

	inline ChaosVector3 GetRainColor()
	{
		auto const rainAddress = GetRainAddress();

		if (!rainAddress)
			return ChaosVector3(Vector3());

		return *reinterpret_cast<ChaosVector3 *>(rainAddress + 0xd0);
	}

	inline void SetRainColor(const ChaosVector3 &color)
	{
		auto const rainAddress = GetRainAddress();

		if (!rainAddress)
			return;

		*reinterpret_cast<ChaosVector3 *>(rainAddress + 0xd0) = color;
	}

	inline void SetRainColor(float r, float g, float b)
	{
		SetRainColor(ChaosVector3(Vector3(r, g, b)));
	}

	inline float GetRainGravity()
	{
		auto const rainAddress = GetRainAddress();

		if (!rainAddress)
			return 0.f;

		return *reinterpret_cast<float *>(rainAddress + 0xc);
	}

	inline void SetRainGravity(float gravity)
	{
		auto const rainAddress = GetRainAddress();

		if (!rainAddress)
			return;

		*reinterpret_cast<float *>(rainAddress + 0xc) = gravity;
	}

	inline float GetRainLight()
	{
		auto const rainAddress = GetRainAddress();

		if (!rainAddress)
			return 0.f;

		return *reinterpret_cast<float *>(rainAddress + 0x150);
	}

	inline void SetRainLight(float light)
	{
		auto const rainAddress = GetRainAddress();

		if (!rainAddress)
			return;

		*reinterpret_cast<float *>(rainAddress + 0x150) = light;
	}
}