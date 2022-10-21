#pragma once

#include "Util/Natives.h"

/*
  _   _  ______  ______  _____    _____     _____   ______  ______        _____  _______  ____   _____   _____  _   _   _____ 
 | \ | ||  ____||  ____||  __ \  / ____|   |  __ \ |  ____||  ____|/\    / ____||__   __|/ __ \ |  __ \ |_   _|| \ | | / ____|
 |  \| || |__   | |__   | |  | || (___     | |__) || |__   | |__  /  \  | |        | |  | |  | || |__) |  | |  |  \| || |  __ 
 | . ` ||  __|  |  __|  | |  | | \___ \    |  _  / |  __|  |  __|/ /\ \ | |        | |  | |  | ||  _  /   | |  | . ` || | |_ |
 | |\  || |____ | |____ | |__| | ____) |   | | \ \ | |____ | |  / ____ \| |____    | |  | |__| || | \ \  _| |_ | |\  || |__| |
 |_| \_||______||______||_____/ |_____/    |_|  \_\|______||_| /_/    \_\\_____|   |_|   \____/ |_|  \_\|_____||_| \_| \_____|
*/

namespace Memory
{

	inline Vector3 GetVector3(uintptr_t offset)
	{
		return Vector3(*reinterpret_cast<float *>(offset), *reinterpret_cast<float *>(offset + 0x4),
		               *reinterpret_cast<float *>(offset + 0x8));
	}

	inline void SetVector3(uintptr_t offset, Vector3 vec)
	{
		*reinterpret_cast<float *>(offset)       = vec.x;
		*reinterpret_cast<float *>(offset + 0x4) = vec.y;
		*reinterpret_cast<float *>(offset + 0x8) = vec.z;
	}
}