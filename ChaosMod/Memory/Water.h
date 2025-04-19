#pragma once

struct CWaterQuad
{
	short MinX;     // 0x00
	short MinY;     // 0x02
	short MaxX;     // 0x04
	short MaxY;     // 0x06
	uint32_t Alpha; // 0x08
	float Unk0;     // 0x0C
	float Unk1;     // 0x10
	float Z;        // 0x14
	union           // Bitfield Flags
	{
		uint8_t Flags; // 0x18
		struct
		{
			uint8_t UnkFlag0 : 1;  // Bit 0
			uint8_t UnkFlag1 : 1;  // Bit 1
			uint8_t IsAudible : 1; // Bit 2
			uint8_t IsVisible : 1; // Bit 3
			uint8_t UnkFlag4 : 1;  // Bit 4
			uint8_t UnkFlag5 : 1;  // Bit 5
			uint8_t UnkFlag6 : 1;  // Bit 6
			uint8_t UnkFlag7 : 1;  // Bit 7
		};
	};
	uint8_t Type; // 0x19
	char Pad_01A; // 0x1A
	char Pad_01B; // 0x1B
};
static_assert(sizeof(CWaterQuad) == 0x1C);

struct CCalmingQuad
{
	short MinX; // 0x00
	short MinY; // 0x02
	short MaxX; // 0x04
	short MaxY; // 0x06
	float Unk0; // 0x08
};
static_assert(sizeof(CCalmingQuad) == 0xC);

struct CWaveQuad
{
	short MinX;    // 0x00
	short MinY;    // 0x02
	short MaxX;    // 0x04
	short MaxY;    // 0x06
	uint16_t Unk0; // 0x08
	uint8_t X;     // 0x0A
	uint8_t Y;     // 0x0B
};
static_assert(sizeof(CWaveQuad) == 0xC);

namespace Memory
{
	inline static CWaterQuad *GetAllWaterQuads()
	{
		static Handle handle =
		    Memory::FindPattern("4C 8B 05 ? ? ? ? 42 8B 04 03", "48 8B 15 ? ? ? ? 0F 57 C0 F3 44 0F 10 05");
		if (!handle.IsValid())
			return nullptr;

		return *handle.At(2).Into().Get<CWaterQuad *>();
	}

	inline static CCalmingQuad *GetAllCalmingQuads()
	{
		static Handle handle =
		    Memory::FindPattern("4C 8B 05 ? ? ? ? 49 83 C1 0C", "48 89 05 ? ? ? ? 66 89 3D ? ? ? ? 31 C9");
		if (!handle.IsValid())
			return nullptr;

		return *handle.At(2).Into().Get<CCalmingQuad *>();
	}

	inline static CWaveQuad *GetAllWaveQuads()
	{
		static Handle handle =
		    Memory::FindPattern("4C 8B 05 ? ? ? ? 49 83 C1 18", "48 89 05 ? ? ? ? 66 89 35 ? ? ? ? 48 C1 E6 03");
		if (!handle.IsValid())
			return nullptr;

		return *handle.At(2).Into().Get<CWaveQuad *>();
	}

	inline bool IsPedWet(const Ped ped)
	{
		auto const entityAddress = Memory::GetScriptHandleBaseAddress(ped);
		if (!entityAddress)
			return false;
		auto const wetness = *reinterpret_cast<float *>(entityAddress + 0x31C);

		return wetness > 0.005;
	}

	inline void SetWaterCollisionForPed(const Ped ped, const bool toggle)
	{
		auto const entityAddress = Memory::GetScriptHandleBaseAddress(ped);
		if (!entityAddress)
			return;

		auto const navigation = *reinterpret_cast<std::uintptr_t *>(entityAddress + 0x30);
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

	inline CWaterQuad *GetClosestWaterQuad(const Vector3 &pos)
	{
		auto const waterQuads = GetAllWaterQuads();
		if (!waterQuads)
			return nullptr;

		CWaterQuad *closestQuad = nullptr;
		float closestDistance   = std::numeric_limits<float>::max();

		for (int i = 0; i < 821; i++)
		{
			auto const quad     = &waterQuads[i];
			auto const center   = Vector3((quad->MinX + quad->MaxX) / 2.f, (quad->MinY + quad->MaxY) / 2.f, quad->Z);
			auto const distance = pos.DistanceTo(center);
			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestQuad     = quad;
			}
		}

		return closestQuad;
	}
}