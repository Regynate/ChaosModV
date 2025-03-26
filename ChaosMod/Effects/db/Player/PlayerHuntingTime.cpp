#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>
#include "Util/HelpText.h"

static int GetCharacterSlot()
{
	auto const player = PLAYER_PED_ID();
	auto const model  = GET_ENTITY_MODEL(player);

	if (model == GET_HASH_KEY("player_zero"))
		return 0;
	if (model == GET_HASH_KEY("player_one"))
		return 1;
	if (model == GET_HASH_KEY("player_two"))
		return 2;

	return -1;
}

static int GetPlayerMoney()
{
	auto const charSlot = GetCharacterSlot();
	if (charSlot == -1)
		return 0;

	auto const statName = (charSlot == 0) ? "SP0_TOTAL_CASH" : (charSlot == 1) ? "SP1_TOTAL_CASH" : "SP2_TOTAL_CASH";

	int money;
	if (STAT_GET_INT(GET_HASH_KEY(statName), &money, -1))
		return money;
	return 0;
}

static void SetPlayerMoney(const int amount)
{
	auto const charSlot = GetCharacterSlot();
	if (charSlot == -1)
		return;

	auto const statName = (charSlot == 0) ? "SP0_TOTAL_CASH" : (charSlot == 1) ? "SP1_TOTAL_CASH" : "SP2_TOTAL_CASH";


	STAT_SET_INT(GET_HASH_KEY(statName), amount, true);
}

static void GivePlayerMoney(const int amount)
{
	auto const currentMoney = GetPlayerMoney();
	SetPlayerMoney(currentMoney + amount);
}

static bool RequestControlEntity(Entity entity)
{
	if (!DOES_ENTITY_EXIST(entity))
		return false;
	return NETWORK_HAS_CONTROL_OF_ENTITY(entity);
}

static void DeleteEntity(Entity entity)
{
	if (!RequestControlEntity(entity))
		return;
	if (!IS_ENTITY_A_MISSION_ENTITY(entity))
		SET_ENTITY_AS_MISSION_ENTITY(entity, true, true);
	DELETE_ENTITY(&entity);
}

CHAOS_VAR int targetEntity     = 0;
CHAOS_VAR int cletus        = 0;
CHAOS_VAR int countdownTimeMs  = 30000;
CHAOS_VAR int blip             = 0;
CHAOS_VAR bool effectCompleted = false;

static Vector3 GetRandomSpawnLocation(const Ped player)
{
	auto const playerCoords   = GET_ENTITY_COORDS(player, true);
	auto const randomAngle    = static_cast<float>(GET_RANDOM_FLOAT_IN_RANGE(0.0f, 360.0f));
	auto const randomDistance = static_cast<float>(GET_RANDOM_FLOAT_IN_RANGE(50.0f, 100.0f));

	return { playerCoords.x + (randomDistance * cos(randomAngle)), playerCoords.y + (randomDistance * sin(randomAngle)),
		     playerCoords.z + 1.0f };
}

static void SpawnHuntingEntities()
{
	auto const cletusModel = GET_HASH_KEY("csb_cletus");
	auto const deerModel   = GET_HASH_KEY("a_c_deer");

	LoadModel(cletusModel);
	LoadModel(deerModel);
	auto const player       = PLAYER_PED_ID();
	auto const playerCoords = GET_ENTITY_COORDS(player, true);
	Vector3 cletusSpawn     = { playerCoords.x + 2.0f, playerCoords.y + 2.0f, playerCoords.z };

	cletus = CreatePoolPed(26, cletusModel, cletusSpawn.x, cletusSpawn.y, cletusSpawn.z, 0.0f);
	SET_PED_AS_GROUP_MEMBER(cletus, GET_PLAYER_GROUP(PLAYER_ID()));

	auto const spawnCoords = GetRandomSpawnLocation(player);
	targetEntity           = CreatePoolPed(28, deerModel, spawnCoords.x, spawnCoords.y, spawnCoords.z, 0.0f);
	SET_ENTITY_AS_MISSION_ENTITY(targetEntity, true, true);

	blip = ADD_BLIP_FOR_ENTITY(targetEntity);
	SET_BLIP_COLOUR(blip, 1);
	BEGIN_TEXT_COMMAND_SET_BLIP_NAME("STRING");
	BEGIN_TEXT_COMMAND_SCALEFORM_STRING("Hunting Target");
	END_TEXT_COMMAND_SET_BLIP_NAME(blip);
}

static void OnStart()
{
	if (effectCompleted)
		return;
	auto const player = PLAYER_PED_ID();
	SpawnHuntingEntities();
	countdownTimeMs = 25000;
	effectCompleted = false;

	GIVE_WEAPON_TO_PED(player, GET_HASH_KEY("weapon_sniperrifle"), 30, true, true);
}

static void OnStop()
{
	if (DOES_ENTITY_EXIST(targetEntity))
		DeleteEntity(targetEntity);
	if (DOES_ENTITY_EXIST(cletus))
		DeleteEntity(cletus);
	if (DOES_BLIP_EXIST(blip))
		REMOVE_BLIP(&blip);
	effectCompleted = false;
}

static void OnTick()
{
	if (effectCompleted)
		return;

	auto const remainingTime = countdownTimeMs / 1000;

	DisplayHelpText(std::format("Hunt down and kill the dear in {} seconds... or else", remainingTime));

	auto const player = PLAYER_PED_ID();
	if (!DOES_ENTITY_EXIST(targetEntity) || IS_PED_DEAD_OR_DYING(targetEntity, false))
	{
		effectCompleted = true;
		GivePlayerMoney(1000);
		DisplayHelpText("Congratulations! Here's $1000");
		PLAY_SOUND_FRONTEND(-1, "PROPERTY_PURCHASE", "HUD_AWARDS", false);
		REMOVE_BLIP(&blip);
		return;
	}
	countdownTimeMs -= static_cast<int>(GET_FRAME_TIME() * 1000);
	if (countdownTimeMs <= 0)
	{
		effectCompleted = true;
		SET_ENTITY_HEALTH(player, 0, 0);
		REMOVE_BLIP(&blip);
	}
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Hunting Time", 
        .Id = "player_hunting_time", 
        .IsTimed = true,
		.IsShortDuration = true
    }
);