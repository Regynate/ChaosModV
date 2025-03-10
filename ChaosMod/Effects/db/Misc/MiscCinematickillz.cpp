#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static void ped(void (*processor)(const std::int32_t))
{
	static constexpr std::int32_t MAX_ENTITIES = 100;
	std::int32_t peds[MAX_ENTITIES];
	auto const pedCount = worldGetAllPeds(peds, MAX_ENTITIES);

	for (auto const i : std::ranges::iota_view { 0, pedCount })
	{
		auto const pedHandle = peds[i];

		if (!DOES_ENTITY_EXIST(pedHandle))
			continue;

		processor(pedHandle);
	}
}

static int cinematicCamera             = 0;
static bool isCinematicActive          = false;
static auto constexpr slowMotionFactor = 0.2f;
static auto constexpr triggerChance    = 25;
static std::vector<std::int32_t> processedPeds;

static void StartCinematicKill(const std::int32_t ped)
{
	if (isCinematicActive)
		return;

	auto const randomChance = GET_RANDOM_INT_IN_RANGE(0, 100);
	if (randomChance > triggerChance)
		return;

	if (std::ranges::find(processedPeds, ped) != processedPeds.end())
		return;

	processedPeds.emplace_back(ped);

	auto const pedCoords = GET_ENTITY_COORDS(ped, true);

	cinematicCamera      = CREATE_CAM(const_cast<char *>("DEFAULT_SCRIPTED_CAMERA"), true);
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

static void PedsDiedToPlayer(const std::int32_t ped)
{
	auto const isEntityDead        = IS_PED_DEAD_OR_DYING(ped, false);
	auto const hasPlayerDamagedPed = HAS_PLAYER_DAMAGED_AT_LEAST_ONE_PED(PLAYER_ID());

	if (!isEntityDead || !hasPlayerDamagedPed)
		return;
	auto const player        = PLAYER::PLAYER_PED_ID();
	auto const sourceOfDeath = GET_PED_SOURCE_OF_DEATH(ped);
	if (sourceOfDeath != player)
		return;

	StartCinematicKill(ped);
}
static void OnTick()
{
	if (isCinematicActive)
		return;

	ped(PedsDiedToPlayer);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Cinematic Killz",
        .Id = "misc_cinematic_killz", 
        .IsTimed = true
    }
);