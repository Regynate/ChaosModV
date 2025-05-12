#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

struct TpData
{
	Vector3 position;
	Vector3 velocity;
	Vector3 rotation;
};

CHAOS_VAR int ms_State;
CHAOS_VAR std::map<Ped, Vector3> ms_ToTpPeds;
CHAOS_VAR std::map<Vehicle, TpData> ms_ToTpVehs;

static void OnStart()
{
	ms_State = 0;
	ms_ToTpVehs.clear();
	ms_ToTpVehs.clear();
}

static void OnTickLag()
{
	static int lastTick = 0;
	auto curTick        = GET_GAME_TIMER();

	if (curTick > lastTick + 500)
	{
		lastTick = curTick;

		if (++ms_State == 4)
			ms_State = 0;

		if (ms_State == 2)
		{
			for (Ped ped : GetAllPeds())
			{
				if (!IS_PED_IN_ANY_VEHICLE(ped, true) && !GET_VEHICLE_PED_IS_ENTERING(ped))
				{
					Vector3 pedPos = GET_ENTITY_COORDS(ped, false);

					ms_ToTpPeds.emplace(ped, pedPos);
				}
			}

			for (Vehicle veh : GetAllVehs())
			{
				Vector3 vehPos = GET_ENTITY_COORDS(veh, false);

				ms_ToTpVehs.emplace(veh,
				                    TpData { vehPos, GET_ENTITY_VELOCITY(veh), GET_ENTITY_ROTATION(veh, 2) });
			}
		}
		else if (ms_State == 3)
		{
			// save current camera heading to apply after teleporting
			float camHeading = GET_GAMEPLAY_CAM_RELATIVE_HEADING();

			for (const auto &pair : ms_ToTpVehs)
			{
				const Vehicle &veh = pair.first;

				float forwardSpeed = GET_ENTITY_SPEED(veh);

				// if the vehicle is reversing use a negative forward speed
				if (GET_ENTITY_SPEED_VECTOR(veh, true).y < 0)
					forwardSpeed *= -1;

				const auto &tpPos = pair.second;

				SET_ENTITY_COORDS_NO_OFFSET(veh, tpPos.position.x, tpPos.position.y, tpPos.position.z, false, false, false);

				SET_ENTITY_ROTATION(veh, tpPos.rotation.x, tpPos.rotation.y, tpPos.rotation.z, 2, true);
				SET_ENTITY_VELOCITY(veh, tpPos.velocity.x, tpPos.velocity.y, tpPos.velocity.z);

				SET_VEHICLE_FORWARD_SPEED(veh, forwardSpeed);
			}

			ms_ToTpVehs.clear();

			SET_GAMEPLAY_CAM_RELATIVE_HEADING(camHeading);
		}
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, OnTickLag, 
	{
		.Name = "Lag",
		.Id = "time_lag",
		.IsTimed = true,
		.IsShortDuration = true
	}
);