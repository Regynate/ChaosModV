#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	auto const playerPed  = PLAYER_PED_ID();
	auto const playerPos  = GET_ENTITY_COORDS(playerPed, false);
	auto const playerVel  = GET_ENTITY_VELOCITY(playerPed);
	auto const cameraRot  = GET_GAMEPLAY_CAM_ROT(2);

	auto const coords     = Vector3(playerPos.x - SIN(cameraRot.z) * 100.0 + playerVel.x * 4.0,
	                                playerPos.y + COS(cameraRot.z) * 100.0 + playerVel.y * 4.0, playerPos.z + 25.0);

	auto const blimpModel = GET_HASH_KEY("blimp");
	LoadModel(blimpModel);
	auto const blimp      = CreatePoolVehicle(blimpModel, coords.x, coords.y, coords.z, cameraRot.z - 180.f);

	auto const daveyModel = GET_HASH_KEY("ig_davenorton");
	LoadModel(daveyModel);
	auto const davey = CreatePoolPed(6, daveyModel, coords.x, coords.y, coords.z - 10.0f, 0.0f);

	Hash relationshipGroup;
	ADD_RELATIONSHIP_GROUP("_INVERSE_DAVE", &relationshipGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(0, relationshipGroup, "PLAYER"_hash);
	SET_RELATIONSHIP_BETWEEN_GROUPS(0, "PLAYER"_hash, relationshipGroup);

	SET_PED_RELATIONSHIP_GROUP_HASH(davey, relationshipGroup);
	SET_PED_AS_GROUP_MEMBER(davey, GET_PLAYER_GROUP(PLAYER_ID()));

	SET_VEHICLE_FORWARD_SPEED(blimp, 50.0f);

	SET_PED_INTO_VEHICLE(davey, blimp, -1);
	TASK_LEAVE_VEHICLE(davey, blimp, 4160);

	SET_ENTITY_INVINCIBLE(davey, true);

	WAIT(2000);

	while (DOES_ENTITY_EXIST(davey) && IS_ENTITY_IN_AIR(davey))
	{
		WAIT(1000);
	}

	WAIT(2000);
	
	SET_ENTITY_INVINCIBLE(davey, false);
	PLAY_PED_AMBIENT_SPEECH_NATIVE(davey, "GENERIC_HI", "SPEECH_PARAMS_FORCE_SHOUTED", 1);
	TASK_LOOK_AT_ENTITY(davey, playerPed, -1, 2048, 3);
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Inverse Blimp Strats", 
        .Id = "misc_inverse_blimp_strats", 
        .IsTimed = false
    }
);
