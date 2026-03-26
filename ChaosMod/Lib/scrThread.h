#pragma once

#include "game.h"

using DWORD   = unsigned long;
using DWORD64 = unsigned long long;

namespace rage
{
	// Thanks to rainbomizer for this def!
	struct scrProgram
	{
		static const uint32_t PAGE_SIZE = 0x4000;

		void *vft;
		void *m_PageMap;
		DWORD64 *m_CodeBlocks;
		DWORD m_GlobalsSignature;
		DWORD m_CodeSize;
		DWORD m_ParameterCount;
		DWORD m_StaticCount;
		DWORD m_GlobalCount;
		DWORD m_NativesCount;
		void *m_StaticsPointer;
		void *m_GlobalsPointer;
		void **m_NativesPointer;
		DWORD64 field_0x48;
		DWORD64 field_0x50;
		DWORD m_ScriptHash;
		int field_0x5c;
		void *m_ScriptNamePointer;
		DWORD64 *m_StringBlocks;
		DWORD m_StringSize;
		int field_0x74;
		int field_0x78;
		int field_0x7c;
	};

	class scrThread
	{
	  private:
		struct scrThreadLegacyPre2699
		{
			void *vft;
			DWORD m_ThreadId;
			DWORD m_ScriptHash;
			DWORD dwSomething2;
			DWORD m_IP;
			char pad[152];
			__int64 *m_Stack;
			char pad2[24];
			char m_Name[32];
			char pad3[100];
			char chSomething3;
			char pad4[3];
		};

		struct scrThreadLegacy
		{
			void *vft;
			DWORD m_ThreadId;
			DWORD m_ScriptHash;
			DWORD dwSomething2;
			DWORD m_IP;
			char pad[152];
			__int64 *m_Stack;
			char pad2[24];
			char pad_2699[4];
			char m_Name[32];
			char pad3[100];
			char chSomething3;
			char pad4[3];
		};

		struct scrThreadEnhanced
		{
			void *vft;          // 0x0
			DWORD m_ThreadId;   // 0x8
			char pad0[4];       // 0xC
			DWORD m_ScriptHash; // 0x10
			DWORD dwSomething2; // 0x14
			DWORD m_IP;         // 0x18
			char pad[156];      // 0x1C
			__int64 *m_Stack;   // 0xB8
			char pad2[20];
			char pad_Enhanced[128];
			char m_Name[32];
			char pad3[100];
			char chSomething3;
			char pad4[3];
		};

	  public:
		DWORD GetThreadId() const
		{
			if (IsLegacy())
				return reinterpret_cast<const scrThreadLegacy *>(this)->m_ThreadId;
			else
				return reinterpret_cast<const scrThreadEnhanced *>(this)->m_ThreadId;
		}

		DWORD GetHash() const
		{
			if (IsLegacy())
				return reinterpret_cast<const scrThreadLegacy *>(this)->m_ScriptHash;
			else
				return reinterpret_cast<const scrThreadEnhanced *>(this)->m_ScriptHash;
		}

		const char *GetName() const
		{
			if (getGameVersion() < eGameVersion::VER_1_0_2699_0_STEAM)
				return reinterpret_cast<const scrThreadLegacyPre2699 *>(this)->m_Name;
			else if (IsLegacy())
				return reinterpret_cast<const scrThreadLegacy *>(this)->m_Name;
			else
				return reinterpret_cast<const scrThreadEnhanced *>(this)->m_Name;
		}

		__int64 *GetStack() const
		{
			if (getGameVersion() < eGameVersion::VER_1_0_2699_0_STEAM)
				return reinterpret_cast<const scrThreadLegacyPre2699 *>(this)->m_Stack;
			else if (IsLegacy())
				return reinterpret_cast<const scrThreadLegacy *>(this)->m_Stack;
			else
				return reinterpret_cast<const scrThreadEnhanced *>(this)->m_Stack;
		}
	};
}