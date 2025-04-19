#pragma once
#include "Components/LuaScripts.h"
#include "Memory/Memory.h"

inline void RestoreRainProperties()
{
	static std::once_flag cacheFlag;
	static float rainRed, rainGreen, rainBlue, rainGravity, rainLight;

	static constexpr float multiplier   = 5.f;
	static constexpr float colorScale   = 255.f;

	static auto handle =
	    Memory::FindPattern("48 8D 2D ? ? ? ? 0F 2F C6 0F 97 C0 88 44 24 40 84 C0 74 4A 8B 0D ? ? ? ? 83 CB FF 44");
	if (!handle.IsValid())
		return;

	auto const rainAddress       = handle.At(2).Into().Addr();

	auto const colorRedAddress   = rainAddress + static_cast<std::uint64_t>(0xd0);
	auto const colorGreenAddress = rainAddress + static_cast<std::uint64_t>(0xd4);
	auto const colorBlueAddress  = rainAddress + static_cast<std::uint64_t>(0xd8);
	auto const gravityAddress    = rainAddress + static_cast<std::uint64_t>(0xc);
	auto const lightAddress      = rainAddress + static_cast<std::uint64_t>(0x150);

	if (!colorRedAddress || !colorGreenAddress || !colorBlueAddress || !gravityAddress || !lightAddress)
		return;

	std::call_once(cacheFlag,
	    [&]()
	    {       
		    rainRed     = *reinterpret_cast<float *>(colorRedAddress);
		    rainGreen   = *reinterpret_cast<float *>(colorGreenAddress);
		    rainBlue    = *reinterpret_cast<float *>(colorBlueAddress);
		    rainGravity = *reinterpret_cast<float *>(gravityAddress);
		    rainLight   = *reinterpret_cast<float *>(lightAddress);
	    });

	*reinterpret_cast<float *>(colorRedAddress)   = (rainRed / colorScale) * multiplier;
	*reinterpret_cast<float *>(colorGreenAddress) = (rainGreen / colorScale) * multiplier;
	*reinterpret_cast<float *>(colorBlueAddress)  = (rainBlue / colorScale) * multiplier;
	*reinterpret_cast<float *>(gravityAddress)    = rainGravity;
	*reinterpret_cast<float *>(lightAddress)      = rainLight;
}

inline void SetRainProperties(const float r, const float g, const float b, const float gravity, const float light)
{
	static auto handle = Memory::FindPattern("48 8D 2D ? ? ? ? 0F 2F C6 0F 97 C0 88 44 24 40 84 C0 74 4A 8B 0D ? ? ? ? 83 CB FF 44");
	if (!handle.IsValid())
		return;

	auto const rainAddress = handle.At(2).Into().Addr();

	auto constexpr multiplier = 5.f;
	auto constexpr colorScale = 255.f;

	auto const colorRedAddress   = rainAddress + static_cast<std::uint64_t>(0xd0);
	auto const colorGreenAddress = rainAddress + static_cast<std::uint64_t>(0xd4);
	auto const colorBlueAddress  = rainAddress + static_cast<std::uint64_t>(0xd8);
	auto const gravityAddress    = rainAddress + static_cast<std::uint64_t>(0xc);
	auto const lightAddress      = rainAddress + static_cast<std::uint64_t>(0x150);

	if (!colorRedAddress || !colorGreenAddress || !colorBlueAddress || !gravityAddress || !lightAddress)
		return;

	*reinterpret_cast<float *>(colorRedAddress)   = (r / colorScale) * multiplier;
	*reinterpret_cast<float *>(colorGreenAddress) = (g / colorScale) * multiplier;
	*reinterpret_cast<float *>(colorBlueAddress)  = (b / colorScale) * multiplier;

	*reinterpret_cast<float *>(gravityAddress)    = gravity;
	*reinterpret_cast<float *>(lightAddress)      = light;
}