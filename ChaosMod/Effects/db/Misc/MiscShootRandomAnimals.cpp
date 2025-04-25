#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Util/Camera.h"

static std::array<std::uint32_t, 5> animalModels = { GET_HASH_KEY("a_c_deer"), GET_HASH_KEY("a_c_coyote"),
	                                                 GET_HASH_KEY("a_c_boar"), GET_HASH_KEY("a_c_rabbit_01"),
	                                                 GET_HASH_KEY("a_c_mtlion") };

static void OnTick()
{
	auto const playerPed = PLAYER_PED_ID();
	if (!IS_PED_SHOOTING(playerPed))
		return;

	auto const randomModel = animalModels[GET_RANDOM_INT_IN_RANGE(0, animalModels.size() - 1)];
	LoadModel(randomModel);
	Vector3 camCoords = GET_GAMEPLAY_CAM_COORD();
	Vector3 pedPos    = GET_ENTITY_COORDS(playerPed, false);

	float distCamToPed =
	    GET_DISTANCE_BETWEEN_COORDS(pedPos.x, pedPos.y, pedPos.z, camCoords.x, camCoords.y, camCoords.z, true);

	auto const spawnPos = Util::GetCoordsFromGameplayCam(distCamToPed + .5f);
	auto const spawnRot = GET_GAMEPLAY_CAM_ROT(2);
	auto const animal   = CREATE_PED(28, randomModel, spawnPos.x, spawnPos.y, spawnPos.z, 0.0f, true, false);
	SET_ENTITY_ROTATION(animal, spawnRot.x, spawnRot.y, spawnRot.z, 2, true);

	SET_PED_TO_RAGDOLL(animal, 2000, 2000, 1, false, false, false);

	APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(animal, 1, .0f, 300.f, 0.f, false, true, true, false);
}

// clang-format off
REGISTER_EFFECT(nullptr, nullptr, OnTick, 
    {
        .Name = "Shoot Random Animals", 
        .Id = "misc_shoot_random_animals", 
        .IsTimed = true
    }
);
