#include <stdafx.h>

static void OnStart()
{
	Vehicle playerVeh = GET_VEHICLE_PED_IS_IN(PLAYER_PED_ID(), false);

	REQUEST_NAMED_PTFX_ASSET("scr_xm_orbital");
	REQUEST_NAMED_PTFX_ASSET("scr_xm_orbital_blast");

	int count = 5;

	for (Vehicle veh : GetAllVehs())
	{
		if (veh != playerVeh)
		{
			Vector3 pos = GET_ENTITY_COORDS(veh, false);

			USE_PARTICLE_FX_ASSET("scr_xm_orbital");
			START_NETWORKED_PARTICLE_FX_NON_LOOPED_AT_COORD("scr_xm_orbital_blast", pos.x, pos.y, pos.z, .0f, .0f, .0f, 1.f, 0, 0, 0, 0);
			PLAY_SOUND_FROM_COORD(-1, "DLC_XM_Explosions_Orbital_Cannon", pos.x, pos.y, pos.z, 0, 1, 0, 0);

			ADD_EXPLOSION(pos.x, pos.y, pos.z, 9, 100.f, true, false, 3.f, false);

			// EXPLODE_VEHICLE(veh, true, false);

			if (--count == 0)
			{
				count = 5;

				WAIT(0);
			}
		}
	}
}

static RegisterEffect registerEffect(EFFECT_EXPLODE_VEHS, OnStart, EffectInfo
	{
		.Name = "Explode All Nearby Vehicles",
		.Id = "vehs_explode",
		.IncompatibleWith = { EFFECT_VEHS_INVINCIBLE }
	}
);