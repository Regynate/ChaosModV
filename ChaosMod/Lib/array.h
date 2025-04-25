#pragma once

#include "game.h"

using DWORD64 = unsigned long long;
using DWORD   = unsigned long;
using WORD    = unsigned short;

namespace rage
{
	template <typename T> struct array
	{
		T *elements;
		WORD count;
	};
}