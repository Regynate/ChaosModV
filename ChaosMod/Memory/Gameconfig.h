#pragma once

#include "game.h"

#include "Handle.h"
#include "Memory.h"

namespace Memory
{
    inline const uintptr_t GetGameConfig()
    {
        // same on Legacy and Enhanced
        static auto handle = FindPattern("48 8B 0D ?? ?? ?? ?? BA B8 8E 9C 53");
        if (!handle.IsValid())
        {
            return 0;
        }

        static Handle configManager__instance = handle.At(2).Into().Value<uintptr_t>();

        return configManager__instance.At(0x18).Value<uintptr_t>();
    }

    inline void SetCrouching(bool state)
    {
        static auto handle = FindPattern("83 B9 ? ? 00 00 00 74 ? BA 0E 00 00 00 E9", "83 B8 ? ? 00 00 00 0F 84 ? ? ? ? 48 89 F1 BA 0E 00 00 00");

        if (!handle.IsValid())
            return;

        static int allowCrouchedMovementOffset = handle.At(2).Value<int>();

        auto gameConfig = GetGameConfig();
        if (!gameConfig)
            return;

        // m_AllowCrouchedMovement
        *reinterpret_cast<unsigned long*>(gameConfig + allowCrouchedMovementOffset) = state;
        // m_AllowStealthMovement
        *reinterpret_cast<unsigned long*>(gameConfig + allowCrouchedMovementOffset + 8) = !state;
    }
}