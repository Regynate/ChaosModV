#pragma once

using Hash = unsigned long;

namespace Util
{
	inline Hash GetPedWeapon(Ped ped)
	{
		// TODO: Get ped weapon from CWeaponInfo
		Hash wepHash;
		GET_CURRENT_PED_WEAPON(ped, &wepHash, true);
		return wepHash;
	}
}