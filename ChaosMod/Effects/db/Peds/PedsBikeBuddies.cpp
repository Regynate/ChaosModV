#include <stdafx.h>
#include "Util/Vehicle.h"

// Effect by Reguas

static void OnStart()
{
	static const Hash model = GET_HASH_KEY("vader");
	LoadModel(model);
	SetSurroundingPedsInVehicles(model, 120);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, EffectInfo
    {
        .Name = "Bike Buddies",
        .Id = "peds_bike_bois",
		.EffectGroupType = EEffectGroupType::TrafficSpawner
    }
);