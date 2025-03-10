#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static constexpr int totalHospitals = 5;

static void SetRandomHospital()
{
	auto const randomHospital = GET_RANDOM_INT_IN_RANGE(0, totalHospitals);
	for (int i = 0; i < totalHospitals; i++)
	{
		bool enable = (i == randomHospital);
		DISABLE_HOSPITAL_RESTART(i, !enable);
	}
}

static void RestoreHospitals()
{
	for (int i = 0; i < totalHospitals; i++)
		DISABLE_HOSPITAL_RESTART(i, false);
}

static void OnStart()
{
	SetRandomHospital();
}

static void OnStop()
{
	RestoreHospitals();
}

static void OnTick()
{
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, nullptr, 
    {
        .Name = "Random Hospital", 
        .Id = "misc_random_hospital", 
        .IsTimed = true
    }
);
