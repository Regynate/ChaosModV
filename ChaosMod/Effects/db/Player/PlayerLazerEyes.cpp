#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Util/Camera.h"

static void DrawLine(Vector3 startPosition, Vector3 endPosition)
{
	DRAW_LINE(startPosition.x, startPosition.y, startPosition.z, endPosition.x, endPosition.y, endPosition.z, 255, 0, 0,
	          200);
}

CHAOS_VAR std::list<Ped> pedsOnFire {};

static void StartFire(Ped ped)
{
	if (IS_ENTITY_ON_FIRE(ped))
		return;

	while (pedsOnFire.size() > 10)
	{
		STOP_ENTITY_FIRE(pedsOnFire.front());
		pedsOnFire.pop_front();
	}

	pedsOnFire.push_back(ped);
	START_ENTITY_FIRE(ped);
}

static void OnTick()
{
	auto const playerPed     = PLAYER_PED_ID();
	auto const playerCoords  = GET_ENTITY_COORDS(playerPed, false);
	auto const camRot        = GET_GAMEPLAY_CAM_ROT(2);

	auto constexpr HEAD      = 31086;

	auto const eyeCoords1    = GET_PED_BONE_COORDS(playerPed, HEAD, 0.06f, 0.1f, 0.05f);
	auto const eyeCoords2    = GET_PED_BONE_COORDS(playerPed, HEAD, 0.06f, 0.1f, -0.05f);

	auto const headRot       = invoke<Vector3>(0xCE6294A232D03786, playerPed, GET_PED_BONE_INDEX(playerPed, HEAD));

	auto const direction     = (IS_AIM_CAM_ACTIVE() ? camRot : headRot).GetDirectionForRotation() * 200.f;

	auto const targetCoords1 = eyeCoords1 + direction;
	auto const targetCoords2 = eyeCoords2 + direction;

	DrawLine(eyeCoords1, targetCoords1);
	DrawLine(eyeCoords2, targetCoords2);

	auto const ignoreEntity = IS_PED_IN_ANY_VEHICLE(playerPed, false) ? GET_VEHICLE_PED_IS_IN(playerPed, false) : playerPed;

	auto const raycast = START_EXPENSIVE_SYNCHRONOUS_SHAPE_TEST_LOS_PROBE(
	    eyeCoords1.x, eyeCoords1.y, eyeCoords1.z, targetCoords1.x, targetCoords1.y, targetCoords1.z, 511, ignoreEntity, 0);

	BOOL hit {};
	Vector3 hitCoords {};
	Vector3 surface {};
	Entity hitEntity {};

	if (GET_SHAPE_TEST_RESULT(raycast, &hit, &hitCoords, &surface, &hitEntity) == 2)
	{
		if (hit)
		{
			if (hitEntity)
			{
				if (IS_ENTITY_A_VEHICLE(hitEntity) && !IS_PED_IN_VEHICLE(playerPed, hitEntity, false))
				{
					for (Ped ped : GetAllPeds())
						if (IS_PED_IN_VEHICLE(ped, hitEntity, false))
							StartFire(ped);
				}
				else if (IS_ENTITY_A_PED(hitEntity))
				{
					StartFire(hitEntity);
				}
			}
			if (!hitCoords.IsDefault())
			{
				USE_PARTICLE_FX_ASSET("core");
				START_PARTICLE_FX_NON_LOOPED_AT_COORD("exp_grd_gren_sp", hitCoords.x, hitCoords.y, hitCoords.z, 0.f,
				                                      0.f, 0.f, 0.2f, 0, 0, 0);
			}
		}
	}
}

static void OnStart()
{
	REQUEST_NAMED_PTFX_ASSET("core");
	while (!HAS_NAMED_PTFX_ASSET_LOADED("core"))
		WAIT(0);

	pedsOnFire.clear();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, OnTick, 
    {
        .Name = "Lazer Eyes", 
        .Id = "player_laser_eyes", 
        .IsTimed = true,
		.IsShortDuration = true
    }
);
