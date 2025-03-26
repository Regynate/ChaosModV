#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

CHAOS_VAR int cinematicCamera   = 0;
CHAOS_VAR bool isCinematicActive = false;
CHAOS_VAR auto constexpr slowMotionFactor = 0.2f;
CHAOS_VAR std::vector<Ped> processedPeds;

static void StartCinematicKill(const Ped ped)
{
	if (isCinematicActive)
		return;

	if (std::ranges::find(processedPeds, ped) != processedPeds.end())
		return;

	processedPeds.emplace_back(ped);

	auto const pedCoords = GET_ENTITY_COORDS(ped, true);

	cinematicCamera      = CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", true);
	SET_CAM_COORD(cinematicCamera, pedCoords.x, pedCoords.y, pedCoords.z + 1.0f);
	POINT_CAM_AT_ENTITY(cinematicCamera, ped, 0.0f, 0.0f, 0.0f, false);
	SET_CAM_ACTIVE(cinematicCamera, true);
	RENDER_SCRIPT_CAMS(true, false, 3000, true, false, 0);

	SET_TIME_SCALE(slowMotionFactor);
	isCinematicActive = true;

	WAIT(3000);

	RENDER_SCRIPT_CAMS(false, false, 3000, true, false, 0);
	DESTROY_CAM(cinematicCamera, false);
	SET_TIME_SCALE(1.0f);
	isCinematicActive = false;
}

static void OnStart()
{
	isCinematicActive = false;
	cinematicCamera   = 0;
	processedPeds.clear();
}

static void OnStop()
{
	if (!isCinematicActive)
	{
		processedPeds.clear();
		return;
	}

	RENDER_SCRIPT_CAMS(false, false, 3000, true, false, 0);
	DESTROY_CAM(cinematicCamera, false);
	SET_TIME_SCALE(1.0f);
	isCinematicActive = false;
	processedPeds.clear();
}

static void PedsDiedToPlayer()
{
	for (auto const ped : GetAllPeds())
	{
		auto const isEntityDead        = IS_PED_DEAD_OR_DYING(ped, false);
		auto const hasPlayerDamagedPed = HAS_PLAYER_DAMAGED_AT_LEAST_ONE_PED(PLAYER_ID());

		if (!isEntityDead || !hasPlayerDamagedPed)
			continue;
		auto const player        = PLAYER_PED_ID();
		auto const sourceOfDeath = GET_PED_SOURCE_OF_DEATH(ped);
		if (sourceOfDeath != player)
			continue;
		StartCinematicKill(ped);
	}
}
static void OnTick()
{
	if (isCinematicActive)
		return;

	PedsDiedToPlayer();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Cinematic Killz",
        .Id = "misc_cinematic_killz", 
        .IsTimed = true
    }
);