#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

namespace
{
	struct Vector2
	{
		float x, y;

		Vector2() = default;
		Vector2(float x, float y) : x(x), y(y)
		{
		}

		Vector2 operator+(const Vector2 &other) const
		{
			return { x + other.x, y + other.y };
		}

		Vector2 operator-(const Vector2 &other) const
		{
			return { x - other.x, y - other.y };
		}

		Vector2 operator*(float scalar) const
		{
			return { x * scalar, y * scalar };
		}

		float GetLength() const
		{
			return std::sqrt(x * x + y * y);
		}
	};

	struct Quad
	{
		Vector2 min;
		Vector2 max;
		uint32_t alpha;
		char padding1[0x8];
		float height;
		char padding2[0x4];
	};

	struct QuadArray
	{
		uint64_t quadPool;
		short quadCount;
	};

	constexpr const char *TARGET_MODULE = "GTA5.exe";
	QuadArray *oceanData                = nullptr;

	class Pointer
	{
	  public:
		explicit Pointer(void *handle) : mPointer(handle)
		{
		}
		explicit Pointer(std::uintptr_t handle) : mPointer(reinterpret_cast<void *>(handle))
		{
		}

		Pointer Rip()
		{
			return Add(As<std::int32_t &>()).Add(4U);
		}

		template <typename T> constexpr std::enable_if_t<std::is_pointer_v<T>, T> As()
		{
			return static_cast<T>(mPointer);
		}

		template <typename T> constexpr std::enable_if_t<std::is_lvalue_reference_v<T>, T> As()
		{
			return *static_cast<std::add_pointer_t<std::remove_reference_t<T>>>(mPointer);
		}

		template <typename T> constexpr std::enable_if_t<std::is_same_v<T, std::uintptr_t>, T> As()
		{
			return reinterpret_cast<T>(mPointer);
		}

		template <typename T> Pointer Add(T offset)
		{
			return Pointer(As<std::uintptr_t>() + offset);
		}

	  private:
		void *mPointer;
	};


	static uint64_t PatternScan(const char *moduleName, const char *signature);
	static uint64_t FindPattern(const uint8_t *scanBytes, size_t sizeOfImage, const std::vector<int> &patternBytes);
	static uint8_t *GetScanBytes(const char *moduleName, size_t &sizeOfImage);
	static std::vector<int> PatternToBytes(const char *pattern);

	static void Initialization()
	{
		oceanData = reinterpret_cast<QuadArray *>(PatternScan(TARGET_MODULE, "74 41 4C 8B 05 ? ? ?") + 5);
	}

	static Vector3 GetQuadCoordinates(Quad *quad)
	{
		return { static_cast<float>(quad->min.x + quad->max.x) / 2.0f,
			     static_cast<float>(quad->min.y + quad->max.y) / 2.0f, quad->height };
	}

	static Quad *GetQuadByIndex(short index)
	{
		return reinterpret_cast<Quad *>(oceanData->quadPool + (index * 0x1C));
	}

	static Quad *GetClosestQuad()
	{
		auto const player            = PLAYER_PED_ID();
		auto const playerCoordinates = GET_ENTITY_COORDS(player, true);
		float closestDistance        = FLT_MAX;
		Quad *closestQuad            = nullptr;

		for (auto index = 0; index < oceanData->quadCount; ++index)
		{
			auto const currentQuad     = GetQuadByIndex(index);
			auto const quadCoordinates = GetQuadCoordinates(currentQuad);
			auto const distance =
			    GET_DISTANCE_BETWEEN_COORDS(playerCoordinates.x, playerCoordinates.y, playerCoordinates.z,
			                                quadCoordinates.x, quadCoordinates.y, quadCoordinates.z, false);

			if (distance < closestDistance)
			{
				closestDistance = distance;
				closestQuad     = currentQuad;
			}
		}

		return closestQuad;
	}

	static uint64_t PatternScan(const char *moduleName, const char *signature)
	{
		size_t sizeOfImage {};
		uint8_t *scanBytes = GetScanBytes(moduleName, sizeOfImage);
		if (!scanBytes)
			return 0;
		auto patternBytes = PatternToBytes(signature);
		return FindPattern(scanBytes, sizeOfImage, patternBytes);
	}

	static uint64_t FindPattern(const uint8_t *scanBytes, size_t sizeOfImage, const std::vector<int> &patternBytes)
	{
		size_t size     = patternBytes.size();
		const int *data = patternBytes.data();

		for (size_t i = 0; i <= sizeOfImage - size; ++i)
		{
			bool found = true;
			for (size_t j = 0; j < size; ++j)
			{
				if (scanBytes[i + j] != data[j] && data[j] != -1)
				{
					found = false;
					break;
				}
			}
			if (found)
				return reinterpret_cast<uint64_t>(&scanBytes[i]);
		}
		return 0;
	}

	static uint8_t *GetScanBytes(const char *moduleName, size_t &sizeOfImage)
	{
		auto *moduleHandle = GetModuleHandleA(moduleName);
		if (!moduleHandle)
			return nullptr;

		auto *dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(moduleHandle);
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return nullptr;

		auto *ntHeaders =
		    reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<uint8_t *>(moduleHandle) + dosHeader->e_lfanew);

		if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
			return nullptr;

		sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		return reinterpret_cast<uint8_t *>(moduleHandle);
	}

	static std::vector<int> PatternToBytes(const char *pattern)
	{
		auto bytes  = std::vector<int> {};

		auto *start = const_cast<char *>(pattern);
		auto *end   = const_cast<char *>(pattern) + strlen(pattern);

		for (auto *current = start; current < end; ++current)
		{
			if (*current == '?')
			{
				++current;
				if (*current == '?')
					++current;
				bytes.emplace_back(-1);
			}
			else
			{
				bytes.emplace_back(strtoul(current, &current, 16));
			}
		}
		return bytes;
	}
}

static void OnStart()
{
	Initialization();
	auto const player      = PLAYER_PED_ID();
	auto const closestQuad = GetClosestQuad();
	if (closestQuad)
	{
		auto const closestQuadCoordinates = GetQuadCoordinates(closestQuad);
		SET_ENTITY_COORDS(player, closestQuadCoordinates.x, closestQuadCoordinates.y, closestQuadCoordinates.z - 5,
		                  false, false, false, false);
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Take as Swim", 
        .Id = "player_take_as_swim", 
        .IsTimed = false
    }
);
