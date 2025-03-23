#include <stdafx.h>
#include "Effects/Register/RegisterEffect.h"
#include <ranges>

CHAOS_VAR std::vector<std::uint32_t> OriginalWeaponGroup;

static void BackupWeaponProperties()
{
	auto constexpr weaponInfoRva     = static_cast<std::uintptr_t>(0x2906E40);
	auto constexpr weaponGroupOffset = static_cast<std::uint64_t>(0x5c);

	auto const baseAddress            = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
	auto const absolute                = baseAddress + weaponInfoRva;

	auto const weaponInfo             = *reinterpret_cast<std::uintptr_t ***>(absolute);
	auto const weaponCount            = *reinterpret_cast<std::uint16_t *>(absolute + 0x8);

	OriginalWeaponGroup.clear();

	std::ranges::for_each(std::views::iota(std::uint16_t { 0 }, weaponCount),
	    [&](std::uint16_t i)
	    {
		    auto const weapon = weaponInfo[i];

		    if (!weapon)
			    return;

		    auto const ptr          = reinterpret_cast<std::uintptr_t>(weapon);
		    auto const weaponGroup = ptr + weaponGroupOffset;

		                      
		    OriginalWeaponGroup.push_back(*reinterpret_cast<std::uint32_t *>(weaponGroup));
	    });
}

static void ModifyWeaponProperties()
{
	auto constexpr weaponInfoRva     = static_cast<std::uintptr_t>(0x2906E40);
	auto constexpr weaponGroupOffset = static_cast<std::uint64_t>(0x5c);

	auto const baseAddress            = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
	auto const absolute                = baseAddress + weaponInfoRva;

	auto const weaponInfo             = *reinterpret_cast<std::uintptr_t ***>(absolute);
	auto const weaponCount            = *reinterpret_cast<std::uint16_t *>(absolute + 0x8);

	std::ranges::for_each(std::views::iota(std::uint16_t { 0 }, weaponCount),
	    [&](std::uint16_t i)
	    {
		    auto const weapon = weaponInfo[i];

		    if (!weapon)
			    return;

		    auto const ptr          = reinterpret_cast<std::uintptr_t>(weapon);
		    auto const weaponGroup = ptr + weaponGroupOffset;

		    *reinterpret_cast<std::uint32_t *>(weaponGroup) = 0x18D5FA97;
	    });
}

static void RestoreWeaponProperties()
{
	auto constexpr weaponInfoRva     = static_cast<std::uintptr_t>(0x2906E40);
	auto constexpr weaponGroupOffset = static_cast<std::uint64_t>(0x5c);

	auto const baseAddress            = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
	auto const absolute                = baseAddress + weaponInfoRva;

	auto const weaponInfo             = *reinterpret_cast<std::uintptr_t ***>(absolute);
	auto const weaponCount            = *reinterpret_cast<std::uint16_t *>(absolute + 0x8);

	std::ranges::for_each(std::views::iota(std::uint16_t { 0 }, weaponCount),
	    [&](std::uint16_t i)
	    {
		    auto const weapon = weaponInfo[i];

		    if (!weapon)
			    return;

		    auto const ptr          = reinterpret_cast<std::uintptr_t>(weapon);
		    auto const weaponGroup = ptr + weaponGroupOffset;

		    *reinterpret_cast<std::uint32_t *>(weaponGroup) = OriginalWeaponGroup[i];
	    });
}

static void OnStart()
{
	BackupWeaponProperties();
	ModifyWeaponProperties();
}

static void OnStop()
{
	RestoreWeaponProperties();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr, 
	{
		.Name = "Use Any Weapons while in Vehicles",
		.Id = "player_use_any_weapons_while_in_vehicle",
		.IsTimed = true
	}
);