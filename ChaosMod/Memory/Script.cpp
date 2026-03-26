#include "stdafx.h"

#include "Script.h"

#include "Memory/Handle.h"
#include "Memory/Memory.h"

#include "Util/Logging.h"

using Hash = unsigned long;

namespace Memory
{
	rage::scrProgram *ScriptThreadToProgramLegacy(rage::scrThread *thread)
	{
		if (!thread)
			return nullptr;

		static const Handle sharedHandle = []
		{
			Handle handle = FindPattern("48 8D 0D ? ? ? ? ? 89 2D ? ? ? ? E8 ? ? ? ? 48 8B 8D B0 00 00 00");
			if (!handle.IsValid())
			{
				LOG("scrProgramRegistry::_FindProgramByHash not found");
				return Handle();
			}

			return handle;
		}();

		static auto scrProgramRegistry__FindProgramByHash = []() -> rage::scrProgram *(*)(DWORD64, Hash)
		{
			if (!sharedHandle.IsValid())
				return nullptr;

			return sharedHandle.At(14).Into().Get<rage::scrProgram *(DWORD64, Hash)>();
		}();

		static auto scrProgramDirectory = []() -> DWORD64
		{
			if (!sharedHandle.IsValid())
				return 0;

			return sharedHandle.At(7).At(sharedHandle.At(3).Value<DWORD>()).Addr();
		}();

		if (!sharedHandle.IsValid())
			return nullptr;

		return scrProgramRegistry__FindProgramByHash(scrProgramDirectory, thread->GetHash());
	}

	rage::scrProgram *ScriptThreadToProgramEnhanced(rage::scrThread *thread)
	{
		static const Handle handle = FindPattern("84 C9 74 1B 66 90");
		if (!handle.IsValid())
			return nullptr;

		static const void (*sysCriticalSectionToken__Lock)(DWORD64) = handle.At(-22).Into().Get<const void(DWORD64)>();
		static const void (*sysCriticalSectionToken__Unlock)(DWORD64) = handle.At(40).Into().Get<const void(DWORD64)>();
		const DWORD64 scrProgramRegistry__sm_Token                    = handle.At(-27).Into().Addr();

		byte *s_ProgHash                                              = handle.At(-10).Into().Get<byte>();

		rage::scrProgram *res;

		sysCriticalSectionToken__Lock(scrProgramRegistry__sm_Token);

		unsigned int index = *(s_ProgHash + 1 + (thread->GetHash() & 0x1F));
		while (index)
		{
			auto element = *(reinterpret_cast<rage::scrProgram **>(s_ProgHash + 216) + index);
			if (element->m_ScriptHash == thread->GetHash())
			{
				res = element;
				break;
			}
			index = *(s_ProgHash + index + 33);
		}

		sysCriticalSectionToken__Unlock(scrProgramRegistry__sm_Token);

		return res;
	}

	// Thanks to rainbomizer for this one!
	rage::scrProgram *ScriptThreadToProgram(rage::scrThread *thread)
	{
		return IsLegacy() ? ScriptThreadToProgramLegacy(thread) : ScriptThreadToProgramEnhanced(thread);
	}

	Handle FindScriptPattern(const std::string &pattern, rage::scrProgram *program)
	{
		if (!program)
			return Handle();

		std::uint32_t codeBlocksSize = (program->m_CodeSize + 0x3FFF) >> 14;
		for (std::uint32_t i = 0; i < codeBlocksSize; i++)
		{
			auto handle = Memory::FindPattern(
			    pattern,
			    PatternScanRange { program->m_CodeBlocks[i],
			                       program->m_CodeBlocks[i]
			                           + (i == codeBlocksSize - 1 ? program->m_CodeSize : program->PAGE_SIZE) });
			if (handle.IsValid())
				return handle;
		}

		return Handle();
	}

	DWORD64 *GetGlobalPtr(int globalId)
	{
		static auto globalPtr = []() -> DWORD64 **
		{
			auto handle = FindPattern("4C 8D 05 ? ? ? ? 4D 8B 08 4D 85 C9 74 11", "48 8D 3D ? ? ? ? 31 ED 4C 8D 25");
			if (!handle.IsValid())
				return nullptr;

			return handle.At(2).Into().Get<DWORD64 *>();
		}();

		static auto fallbackToSHV = []() -> bool
		{
			bool fallbackToSHV = !globalPtr;

			if (fallbackToSHV)
			{
				LOG("Warning: _globalPtr not found, falling back to SHV's getGlobalPtr");
			}
			// HACK: Check for the presence some arbitrary module specific to FiveM
			// Also check if player is in a mp session if so to check for FiveM sp
			else if (GetModuleHandle(L"gta-net-five.dll"))
			{
				bool modeDetermined = false;

				auto handle         = FindPattern("48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 74 09 48 8D 15");
				if (handle.IsValid())
				{
					auto _networkObj = handle.At(2).Into().Get<DWORD64>();
					handle           = FindPattern("83 B9 ? ? 00 00 05 0F 85 ? ? ? ? E9");
					if (handle.IsValid())
					{
						fallbackToSHV  = *reinterpret_cast<DWORD *>(*_networkObj + handle.At(2).Value<WORD>()) == 5;
						modeDetermined = true;
					}
				}

				if (!modeDetermined)
				{
					LOG("Warning: FiveM detected but could not determine mode, switching to fallback for GetGlobalPtr");
					fallbackToSHV = true;
				}
			}

			if (fallbackToSHV)
				LOG("Warning: FiveM (non-sp) detected, features such as Failsafe will not work!");

			return fallbackToSHV;
		}();

		return fallbackToSHV ? getGlobalPtr(globalId) : (&globalPtr[globalId >> 18 & 0x3F][globalId & 0x3FFFF]);
	}

	__int64 *GetScriptLocalPtr(int localId, rage::scrThread *thread)
	{
		if (localId < 0 || thread == 0 || thread->GetStack() == 0)
			return nullptr;

		__int64 *localAddress = nullptr;
		localAddress          = thread->GetStack() + localId;
		*localAddress         = (uint)*localAddress;

		return localAddress;
	}
}