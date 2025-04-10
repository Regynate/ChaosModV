#pragma once
#include "Components/LuaScripts.h"
#include "Memory/Memory.h"

inline bool IsPedWet()
{

	static auto handle = Memory::FindPattern("E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? 48 8B 0D ? ? ? ? 8B D7 E8 ? ? ? ? 48 8B F8 48 85 C0");
	if (!handle.IsValid())
		return false;

	auto const function   = handle.Into().Addr();

	auto const getAddress = reinterpret_cast<std::uintptr_t (*)(std::int32_t)>(function);
	if (!getAddress)
		return false;

	auto const entityAddress = reinterpret_cast<std::uintptr_t *>(getAddress(PLAYER_PED_ID()));
	if (!entityAddress)
		return false;

	auto const ped = reinterpret_cast<std::uintptr_t>(entityAddress);
	if (!ped)
		return false;

	auto const wetness = *reinterpret_cast<float *>(ped + 0x31C);
	if (!wetness)
		return false;

	auto const isPedWet = wetness > 0.005;

	return isPedWet;
}

inline void SetWaterCollisionForPlayer(const bool toggle)
{

	static auto handle = Memory::FindPattern("E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? 48 8B 0D ? ? ? ? 8B D7 E8 ? ? ? ? 48 8B F8 48 85 C0");
	if (!handle.IsValid())
		return;

	auto const function = handle.Into().Addr();

	auto const getAddress     = reinterpret_cast<std::uintptr_t (*)(std::int32_t)>(function);
	if (!getAddress)
		return;

	auto const entityAddress = reinterpret_cast<std::uintptr_t *>(getAddress(PLAYER_PED_ID()));
	if (!entityAddress)
		return;

	auto const ped = reinterpret_cast<std::uintptr_t>(entityAddress);
	if (!ped)
		return;

	auto const navigation = *reinterpret_cast<std::uintptr_t *>(ped + 0x30);
	if (!navigation)
		return;

	auto const damp = *reinterpret_cast<std::uintptr_t *>(navigation + 0x10);
	if (!damp)
		return;

	auto const collision = reinterpret_cast<float *>(damp + 0x54);
	if (!collision)
		return;

	*collision = toggle ? 1.f : 0.f;
}

inline Vector3 GetClosestQuad()
{
	struct Quad
	{
		std::int16_t minx;
		std::int16_t miny;
		std::int16_t maxx;
		std::int16_t maxy;
		std::uint32_t alpha;
		char _0x000C[8];
		float height;
		char _0x0024[4];
	};
#undef max

	static auto handle = Memory::FindPattern("? 6B C9 1C ? 03 0D ? ? ? ? 66 ? 03 C5 66 89 05 ? ? ? ?");
	if (!handle.IsValid())
		return {0, 0, 0};

	auto ocean           = handle.At(6).Into().Addr();

	auto const pool      = *reinterpret_cast<std::uint64_t *>(ocean);
	auto const size      = *reinterpret_cast<std::uint16_t *>(ocean + static_cast<std::uintptr_t>(0x8));

	auto const myCoords  = GET_ENTITY_COORDS(PLAYER_PED_ID(), true);
	auto closestDistance = std::numeric_limits<float>::max();
	Quad *closestQuad {};

	constexpr std::array<std::uint16_t, 212> ignoredIndices = {
		293, 292, 279, 289, 492, 547, 221, 219, 220, 223, 172, 176, 181, 182, 176, 179, 178, 119, 118, 127, 125, 124,
		123, 80,  79,  83,  85,  38,  42,  33,  46,  45,  50,  52,  51,  56,  86,  87,  88,  89,  91,  93,  95,  101,
		99,  102, 146, 145, 149, 151, 152, 156, 199, 200, 198, 209, 203, 234, 225, 236, 306, 303, 295, 316, 317, 310,
		387, 386, 395, 392, 455, 494, 512, 514, 517, 516, 563, 557, 555, 608, 607, 650, 706, 604, 648, 716, 711, 712,
		706, 708, 707, 702, 784, 783, 782, 781, 779, 778, 777, 776, 773, 770, 759, 756, 755, 766, 763, 747, 749, 752,
		751, 741, 742, 692, 693, 694, 698, 695, 696, 679, 680, 681, 691, 688, 685, 672, 671, 645, 644, 642, 639, 638,
		637, 636, 635, 633, 634, 632, 630, 631, 629, 628, 629, 577, 575, 576, 573, 572, 625, 624, 571, 574, 570, 524,
		523, 503, 504, 478, 481, 484, 482, 419, 421, 431, 430, 346, 345, 344, 343, 342, 264, 263, 270, 272, 268, 265,
		222, 218, 169, 167, 168, 164, 216, 217, 250, 485, 426, 430, 346, 344, 529, 530, 528, 527, 526, 525, 578, 579,
		580, 536, 432, 347, 339, 420, 273, 271, 534, 358, 581, 457, 646, 553
	};

	for (auto const i : std::views::iota(std::uint16_t(0), size))
	{
		if (std::ranges::find(ignoredIndices, i) != ignoredIndices.end())
			continue;

		auto constexpr offset  = static_cast<std::uint16_t>(0x1C);
		auto const currentQuad = reinterpret_cast<Quad *>(pool + (i * offset));
		if (!currentQuad)
			continue;

		auto const quadCoords =
		    Vector3 { static_cast<float>(currentQuad->minx + currentQuad->maxx) / 2.0f,
			          static_cast<float>(currentQuad->miny + currentQuad->maxy) / 2.0f, currentQuad->height };

		auto const xDiff          = quadCoords.x - myCoords.x;
		auto const yDiff          = quadCoords.y - myCoords.y;
		auto const zDiff          = quadCoords.z - myCoords.z;
		auto const distanceToQuad = std::sqrt(xDiff * xDiff + yDiff * yDiff + zDiff * zDiff);

		if (distanceToQuad < closestDistance)
		{
			closestDistance = distanceToQuad;
			closestQuad     = currentQuad;
		}
	}

	if (!closestQuad)
		return {};

	auto const quadCoords =
	    Vector3 { static_cast<float>(closestQuad->minx + closestQuad->maxx) / 2.0f,
		          static_cast<float>(closestQuad->miny + closestQuad->maxy) / 2.0f, closestQuad->height };

	return Vector3(quadCoords.x, quadCoords.y, quadCoords.z);
}


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