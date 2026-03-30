#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

#include "Components/EffectDispatcher.h"
#include "Components/EntityTracking.h"

CHAOS_VAR const std::string configKey     = "PlaneHitPole";

CHAOS_VAR const std::set<Hash> poleModels = {
	"prop_roadpole_01b"_hash,         "prop_roadpole_01a"_hash,
	"prop_flagpole_3a"_hash,          "prop_flagpole_2c"_hash,
	"prop_flagpole_1a"_hash,          "prop_flagpole_2a"_hash,
	"prop_flagpole_2b"_hash,          "prop_strip_pole_01"_hash,
	"prop_fnclink_05pole"_hash,       "prop_fnccorgm_02pole"_hash,
	"prop_cctv_pole_02"_hash,         "prop_tram_pole_double01"_hash,
	"prop_tram_pole_roadside"_hash,   "prop_tram_pole_double02"_hash,
	"prop_tram_pole_single02"_hash,   "prop_tram_pole_wide01"_hash,
	"prop_cctv_pole_03"_hash,         "prop_cctv_pole_01a"_hash,
	"prop_tram_pole_single01"_hash,   "prop_cctv_pole_04"_hash,
	"prop_tram_pole_double03"_hash,   "prop_fncsec_01pole"_hash,
	"prop_facgate_02pole"_hash,       "prop_fncsec_02pole"_hash,
	"prop_scaffold_pole"_hash,        "prop_dock_woodpole5"_hash,
	"prop_dock_woodpole3"_hash,       "prop_dock_woodpole2"_hash,
	"prop_dock_woodpole4"_hash,       "prop_dock_woodpole1"_hash,
	"prop_traffic_03b"_hash,          "prop_traffic_rail_1a"_hash,
	"prop_traffic_01a"_hash,          "prop_traffic_01d"_hash,
	"prop_traffic_rail_3"_hash,       "prop_traffic_02a"_hash,
	"prop_traffic_03a"_hash,          "prop_traffic_02b"_hash,
	"prop_traffic_rail_2"_hash,       "prop_traffic_01b"_hash,
	"prop_snow_traffic_rail_1a"_hash, "prop_snow_traffic_rail_1b"_hash,
	"prop_streetlight_07a"_hash,      "prop_streetlight_11a"_hash,
	"prop_streetlight_05"_hash,       "prop_streetlight_11c"_hash,
	"prop_streetlight_12b"_hash,      "prop_streetlight_06"_hash,
	"prop_streetlight_03d"_hash,      "prop_streetlight_04"_hash,
	"prop_streetlight_14a"_hash,      "prop_streetlight_16a"_hash,
	"prop_streetlight_05_b"_hash,     "prop_streetlight_12a"_hash,
	"prop_streetlight_03e"_hash,      "prop_streetlight_07b"_hash,
	"prop_streetlight_15a"_hash,      "prop_streetlight_01b"_hash,
	"prop_streetlight_08"_hash,       "prop_streetlight_01"_hash,
	"prop_streetlight_11b"_hash,      "prop_streetlight_10"_hash,
	"prop_streetlight_03b"_hash,      "prop_streetlight_03c"_hash,
	"prop_streetlight_03"_hash,       "prop_streetlight_02"_hash,
	"prop_streetlight_09"_hash,       "prop_snow_streetlight01"_hash,
	"prop_snow_streetlight_09"_hash,  "prop_snow_streetlight_01_frag_"_hash,
};

CHAOS_VAR const std::vector<Hash> planeModels = {
	"alkonost"_hash,  "alphaz1"_hash,     "avenger"_hash,     "avenger2"_hash, "avenger3"_hash,  "avenger4"_hash,
	"bombushka"_hash, "cargoplane"_hash,  "cargoplane2"_hash, "cuban800"_hash, "dodo"_hash,      "duster"_hash,
	"howard"_hash,    "hydra"_hash,       "jet"_hash,         "lazer"_hash,    "luxor"_hash,     "luxor2"_hash,
	"mammatus"_hash,  "microlight"_hash,  "miljet"_hash,      "mogul"_hash,    "molotok"_hash,   "nimbus"_hash,
	"nokota"_hash,    "pyro"_hash,        "raiju"_hash,       "rogue"_hash,    "seabreeze"_hash, "shamal"_hash,
	"starling"_hash,  "streamer216"_hash, "strikeforce"_hash, "stunt"_hash,    "titan"_hash,     "tula"_hash,
	"velum"_hash,     "velum2"_hash,      "vestra"_hash,      "volatol"_hash,
};

static bool Tracker()
{
	const auto playerPed = PLAYER_PED_ID();

	if (!IS_PED_IN_ANY_VEHICLE(playerPed, false))
	{
		return true;
	}

	const auto vehicle = GET_VEHICLE_PED_IS_IN(playerPed, false);

	for (const auto &entity : GetAllProps())
	{
		if (poleModels.contains(GET_ENTITY_MODEL(entity)) && IS_ENTITY_TOUCHING_ENTITY(vehicle, entity))
		{
			if (ComponentExists<EffectDispatcher>())
			{
				GetComponent<EffectDispatcher>()->DispatchEffect(
				    EffectIdentifier("misc_pole_hit_plane"), EffectDispatcher::DispatchEffectFlag_None, "", "spawn");
				return false;
			}
		}
	}

	return true;
}

static void OnStart()
{
	if (CurrentEffect::GetDispatchContext() == "spawn")
	{
		const auto playerPed = PLAYER_PED_ID();
		const auto playerPos = GET_ENTITY_COORDS(playerPed, false);
		const auto playerDir = GET_ENTITY_FORWARD_VECTOR(playerPed);

		auto coords = Vector3(playerPos.x + playerDir.x * 5.f, playerPos.y + playerDir.y * 5.f, playerPos.z + playerDir.z * 5.f);
		const auto plane = CreatePoolVehicle(planeModels[g_Random.GetRandomInt(0, planeModels.size() - 1)], coords.x,
		                                 coords.y, coords.z, GET_ENTITY_HEADING(playerPed));
	}
	else
	{
		if (ComponentExists<Tracking>())
			GetComponent<Tracking>()->AddTracker(Tracker, configKey);
	}
}

static void OnInit()
{
	if (ComponentExists<Tracking>())
		GetComponent<Tracking>()->AddTrackerByConfigValue(configKey, Tracker);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, OnInit,
	{
		.Name = "Spawn A Plane Next Time You Hit A Pole",
		.Id = "misc_pole_hit_plane"
	}
);