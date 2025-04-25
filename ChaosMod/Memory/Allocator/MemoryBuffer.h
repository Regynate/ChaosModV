#pragma once

// borrowed from MinHook

#include <windows.h>

#if defined(_M_X64) || defined(__x86_64__)
    #define MEMORY_SLOT_SIZE 64
#else
    #define MEMORY_SLOT_SIZE 32
#endif

LPVOID AllocateBuffer(LPVOID pOrigin);
VOID FreeBuffer(LPVOID pBuffer);