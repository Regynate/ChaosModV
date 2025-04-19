#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

CHAOS_VAR std::vector<Ped> spawnedPeds;
CHAOS_VAR std::array<Hash, 6> protestModels = { GET_HASH_KEY("a_m_m_prolhost_01"), GET_HASH_KEY("a_f_y_hipster_02"),
	                                        GET_HASH_KEY("a_m_y_business_01"), GET_HASH_KEY("a_m_y_genstreet_01"),
	                                        GET_HASH_KEY("a_f_y_vinewood_01"), GET_HASH_KEY("a_m_y_hipster_01") };

CHAOS_VAR std::array<Hash, 2> signModels = { GET_HASH_KEY("prop_sign_road_01a"), GET_HASH_KEY("prop_sign_road_02a") };

static int GetRandomPedModel()
{
	auto const randomIndex = GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(protestModels.size()));
	return protestModels[randomIndex];
}

static int GetRandomSignModel()
{
	auto const randomIndex = GET_RANDOM_INT_IN_RANGE(0, static_cast<int>(signModels.size()));
	return signModels[randomIndex];
}

static void SpawnProtest()
{
	auto const player         = PLAYER_PED_ID();
	auto const playerPosition = GET_ENTITY_COORDS(player, true);
	auto const forwardVector  = GET_ENTITY_FORWARD_VECTOR(player);

	auto const maxProtesters  = GET_RANDOM_INT_IN_RANGE(6, 12);
	constexpr float spacing   = 1.5f;

	for (int i = 0; i < maxProtesters; i++)
	{
		auto const pedModel = GetRandomPedModel();
		LoadModel(pedModel);

		float xOffset = (i % 3) * spacing - 2.0f;
		float yOffset = (i / 3) * spacing;

		Vector3 spawnPosition { playerPosition.x + forwardVector.x * 10.0f + xOffset,
			                    playerPosition.y + forwardVector.y * 10.0f + yOffset, playerPosition.z };

		auto const heading = GET_ENTITY_HEADING(player) - 180;
		auto const ped =
		    CreatePoolPed(4, pedModel, spawnPosition.x, spawnPosition.y, spawnPosition.z, heading);

		SET_ENTITY_AS_MISSION_ENTITY(ped, true, true);
		TASK_START_SCENARIO_IN_PLACE(ped, "WORLD_HUMAN_CHEERING", 0, true);

		auto const shouldHoldSign = GET_RANDOM_INT_IN_RANGE(0, 2);
		if (shouldHoldSign)
		{
			auto const signModel = GetRandomSignModel();
			LoadModel(signModel);

			auto const sign =
			    CreatePoolProp(signModel, spawnPosition.x, spawnPosition.y, spawnPosition.z, false);
			ATTACH_ENTITY_TO_ENTITY(sign, ped, GET_PED_BONE_INDEX(ped, 57005), 0.1f, 0.02f, 0.0f, 0.0f, 0.0f, 0.0f,
			                        false, false, false, false, 2, true);
		}

		spawnedPeds.emplace_back(ped);
	}
}

static void OnStart()
{
	SpawnProtest();
}

// clang-format off
REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Peaceful Protest", 
        .Id = "peds_peaceful_protest", 
        .IsTimed = true
    }
);