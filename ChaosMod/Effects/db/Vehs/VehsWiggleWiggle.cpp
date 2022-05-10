/*
	Effect by Reguas, special for Moxi
*/

#include <stdafx.h>
#include <Memory/ViewMatrix.h>

struct VehicleParams
{
	float savedHeading;
	float savedDelta;
	float multiplier;
	float defaultForwardLength;
	Memory::ViewMatrix vm1;
	Memory::ViewMatrix vm2;

	VehicleParams(Vehicle veh) :
		savedHeading(GET_ENTITY_HEADING(veh)),
		savedDelta(0),
		multiplier(1.f),
		vm1(Memory::ViewMatrix(veh)),
		vm2(Memory::ViewMatrix::ViewMatrix2(veh)) 
	{
		defaultForwardLength = vm1.IsValid() ? vm1.ForwardVector().Length() : 0;
	}

	VehicleParams() : VehicleParams(0) {}
};

static void MultiplyMatrix(Memory::ViewMatrix& vm, float defaultLength, float multiplier)
{
	if (!vm.IsValid())
	{
		return;
	}

	if (abs(vm.ForwardVector().Length() - defaultLength * multiplier) > 0.01f)
	{
		vm.SetForwardVector(vm.ForwardVector() * multiplier);
		vm.SetRightVector(vm.RightVector() * multiplier);
		vm.SetUpVector(vm.UpVector() * multiplier);
	}
}

static void UpdateSize(Vehicle vehicle, VehicleParams& params, bool shouldUpdateHeading)
{
	if (IS_THIS_MODEL_A_CAR(GET_ENTITY_MODEL(vehicle))) // unfortunately, doesn't work with bikes, and has much less effect on planes
	{
		float heading = GET_ENTITY_HEADING(vehicle);
		if (shouldUpdateHeading)
		{
			params.savedHeading = heading;
		}

		float delta = heading - params.savedHeading;
		if (delta < -180.f)
		{
			delta += 360.f;
		}
		if (delta > 180.f)
		{
			delta -= 360.f;
		}

		if (params.savedDelta == 0 && params.savedHeading != 0)
		{
			params.savedDelta = delta;
		}
		if (delta * params.savedDelta < 0 && abs(delta) >= 7.5f)
		{
			params.savedDelta = delta;
			params.savedHeading = heading;
			params.multiplier += 0.05f;
		}

		MultiplyMatrix(params.vm1, params.defaultForwardLength, params.multiplier);
		MultiplyMatrix(params.vm2, params.defaultForwardLength, params.multiplier);
	}
}

static std::map<Vehicle, VehicleParams> vehicles;

static void OnTick()
{
	int currentTick = GET_GAME_TIMER();
	static int lastTick = 0;
	static int lastSizeTick = 0;
	static bool flag = false;

	if (currentTick - lastTick >= 1000)
	{
		for (Vehicle veh : GetAllVehs())
		{
			if (!IS_VEHICLE_SEAT_FREE(veh, -1, false) && GET_ENTITY_SPEED(veh) > 0.1f)
			{
				TASK_VEHICLE_TEMP_ACTION(GET_PED_IN_VEHICLE_SEAT(veh, -1, false), veh, flag ? 7 : 8, 250); // 7 = left; 8 = right
			}
		}
		flag = !flag;
		lastTick = currentTick;
	}

	for (Vehicle veh : GetAllVehs())
	{
		if (!vehicles.contains(veh))
		{
			vehicles.insert(std::make_pair(veh, VehicleParams(veh)));
		}

		UpdateSize(veh, vehicles[veh], currentTick - lastSizeTick >= 501);
	}

	if (currentTick - lastSizeTick >= 501)
	{
		lastSizeTick = currentTick;
	}
}

static void OnStop()
{
	for (Vehicle veh : GetAllVehs())
	{
		VehicleParams params = vehicles[veh];

		MultiplyMatrix(params.vm1, params.defaultForwardLength * params.multiplier, 1.f / params.multiplier);
		MultiplyMatrix(params.vm2, params.defaultForwardLength * params.multiplier, 1.f / params.multiplier);
	}

	vehicles.clear();
}

static RegisterEffect registerEffect(EFFECT_VEHS_WIGGLE_WIGGLE, nullptr, OnStop, OnTick, EffectInfo
	{
		.Name = "Wiggle-Wiggle",
		.Id = "vehs_wiggle_wiggle",
		.IsTimed = true
	}
);
