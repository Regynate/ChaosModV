#pragma once

#include "Lib/scrThread.h"
#include "Memory/Handle.h"

namespace Memory
{
	rage::scrProgram *ScriptThreadToProgramLegacy(rage::scrThread *thread);

	rage::scrProgram *ScriptThreadToProgramEnhanced(rage::scrThread *thread);

	// Thanks to rainbomizer for this one!
	rage::scrProgram *ScriptThreadToProgram(rage::scrThread *thread);

	Handle FindScriptPattern(const std::string &pattern, rage::scrProgram *program);

	DWORD64 *GetGlobalPtr(int globalId);

	__int64 *GetScriptLocalPtr(int localId, rage::scrThread *thread);

	template <typename T> inline T *GetScriptLocal(int localId, rage::scrThread *thread)
	{
		const auto ptr = GetScriptLocalPtr(localId, thread);

		if (!ptr)
			return 0;

		return reinterpret_cast<T *>(ptr);
	}
}