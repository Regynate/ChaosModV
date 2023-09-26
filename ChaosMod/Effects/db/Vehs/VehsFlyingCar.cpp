/*
    Effect by Last0xygen, modified
*/

#include <stdafx.h>

static void OnTick()
{
	Ped player = PLAYER_PED_ID();
	if (IS_PED_IN_ANY_VEHICLE(player, 0))
	{
		Vehicle veh  = GET_VEHICLE_PED_IS_IN(player, 0);

		int vehClass = GET_VEHICLE_CLASS(veh);
		if (vehClass == 15 || vehClass == 16)
		{
			return;
		}

		Vector3 vec        = GET_ENTITY_FORWARD_VECTOR(veh);
		float currentSpeed = GET_ENTITY_SPEED(veh);
		float maxSpeed     = GET_VEHICLE_MODEL_ESTIMATED_MAX_SPEED(GET_ENTITY_MODEL(veh));
		float forwardSpeed = GET_ENTITY_SPEED_VECTOR(veh, true).y;
		
		if (currentSpeed < maxSpeed * 0.3f && GET_ENTITY_HEIGHT_ABOVE_GROUND(veh) < 10.f)
		{
			return;
		}
		
		float deltaSpeed = 10.f * GET_FRAME_TIME();
		Vector3 vel      = GET_ENTITY_VELOCITY(veh);

		if (IS_CONTROL_PRESSED(0, 87)) // Forward
		{
			vel = vec * (forwardSpeed + deltaSpeed);
		}
		else if (IS_CONTROL_PRESSED(0, 88) && forwardSpeed > 0) // Brake
		{
			vel = vec * (forwardSpeed - deltaSpeed);
		}
		else if (forwardSpeed >= maxSpeed * 0.8f)
		{
			vel = vec * (forwardSpeed - deltaSpeed * 0.2f);
		}

		float length = VMAG(vel.x, vel.y, 0);

		if (length > maxSpeed * 3.f)
		{
			vel = vel * (maxSpeed * 3.f / length);
		}

		SET_ENTITY_VELOCITY(veh, vel.x, vel.y, vel.z);

		if (currentSpeed < maxSpeed * 0.4f)
		{
			return;
		}

		float deltaAngle = 100.f * GET_FRAME_TIME();

		Vector3 rot = GET_ENTITY_ROTATION(veh, 2);

		if (IS_CONTROL_PRESSED(0, 89)) // Turn Left
		{
			rot.z += deltaAngle;
		}
		if (IS_CONTROL_PRESSED(0, 90)) // Turn Right
		{
			rot.z -= deltaAngle;
		}

		if (IS_CONTROL_PRESSED(0, 108)) // Roll Left
		{
			rot.y -= deltaAngle;
		}

		if (IS_CONTROL_PRESSED(0, 109)) // Roll Right
		{
			rot.y += deltaAngle;
		}

		if (IS_CONTROL_PRESSED(0, 111)) // Tilt Down
		{
			rot.x -= deltaAngle;
		}

		if (IS_CONTROL_PRESSED(0, 112)) // Tilt Up
		{
			rot.x += deltaAngle;
		}
		SET_ENTITY_ROTATION(veh, rot.x, rot.y, rot.z, 2, 1);
	}
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, EffectInfo
	{
		.Name = "Flying Cars",
		.Id = "vehs_flyingcars",
		.IsTimed = true
	}
);