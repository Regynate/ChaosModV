#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

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

CHAOS_VAR auto constexpr PED_THROWN_OUT_FLAG = 4160;
CHAOS_VAR auto constexpr MUG_AMOUNT       = 7;
CHAOS_VAR auto constexpr PED_TYPE_CIVFEMALE  = 6;
CHAOS_VAR auto constexpr DENISE_MODEL                      = "cs_denise";

static void RemoveSevenDollars()
{
	auto const playerMoney = GetPlayerMoney();
	SetPlayerMoney(playerMoney - MUG_AMOUNT);
}

static Ped SpawnDenise(Vector3 coords)
{
	auto const deniseModelHash = GET_HASH_KEY(DENISE_MODEL);
	LoadModel(deniseModelHash);

	return CREATE_PED(PED_TYPE_CIVFEMALE, deniseModelHash, coords.x + 1.0f, coords.y, coords.z, 0.0f, true, true);
}

static void TriggerDeniseMugging()
{
	auto const playerPed     = PLAYER_PED_ID();
	auto const playerCoords  = GET_ENTITY_COORDS(playerPed, true);
	auto const denisePed     = SpawnDenise(playerCoords);
	auto const playerVehicle = GET_VEHICLE_PED_IS_IN(playerPed, false);

	SET_PED_CAN_RAGDOLL(denisePed, false);
	RemoveSevenDollars();

	if (!IS_PED_IN_ANY_VEHICLE(playerPed, false))
	{
		TASK_REACT_AND_FLEE_PED(denisePed, playerPed);
		return;
	}

	TASK_LEAVE_VEHICLE(playerPed, playerVehicle, PED_THROWN_OUT_FLAG);
	WAIT(1000);

	SET_PED_INTO_VEHICLE(denisePed, playerVehicle, -1);
	WAIT(1000);

	float fleeX = playerCoords.x + ((GET_RANDOM_INT_IN_RANGE(0, 2) == 0) ? -200.0f : 200.0f);
	TASK_VEHICLE_DRIVE_TO_COORD_LONGRANGE(denisePed, playerVehicle, fleeX, playerCoords.y, playerCoords.z, 100.0f,
	                                      786603, 5.0f);
	SET_PED_KEEP_TASK(denisePed, true);
}

static void OnStart()
{
	TriggerDeniseMugging();
}

static void OnStop()
{
}

static void OnTick()
{
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Give Me $7",
        .Id = "peds_give_me_7_dollars", 
    }
);