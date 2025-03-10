#include "Effects/Register/RegisterEffect.h"
#include <stdafx.h>

static std::vector<std::int32_t> recruitedPeds;
static constexpr std::int32_t KEY_E = 0x45; // "E"

static std::int32_t GetAimedPed()
{
	auto const player = PLAYER_PED_ID();
	std::int32_t aimedEntity;
	auto const isAimingAtEntity = GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(PLAYER_ID(), &aimedEntity);
	auto const isPed            = IS_ENTITY_A_PED(aimedEntity);
	auto const isNotInVehicle   = !IS_PED_IN_ANY_VEHICLE(aimedEntity, true);

	if (isAimingAtEntity && isPed && isNotInVehicle)
		return aimedEntity;
	return 0;
}

static void RecruitPed()
{
	if (GetAsyncKeyState(KEY_E))
	{
		auto const ped              = GetAimedPed();
		auto const alreadyRecruited = std::find(recruitedPeds.begin(), recruitedPeds.end(), ped) != recruitedPeds.end();
		if (!ped || alreadyRecruited)
			return;

				static const std::array<std::uint32_t, 3> weaponList = { GET_HASH_KEY("weapon_pistol"),
			                                                         GET_HASH_KEY("weapon_smg"),
			                                                         GET_HASH_KEY("weapon_assaultrifle") };

		auto constexpr max                                       = weaponList.size() - 1;
		auto const randomIndex                                   = GET_RANDOM_INT_IN_RANGE(0, max);
		auto const randomWeapon                                  = weaponList[randomIndex];
		

		auto const group = GET_PED_GROUP_INDEX(PLAYER_PED_ID());
		SET_PED_AS_GROUP_MEMBER(ped, group);
		SET_PED_COMBAT_RANGE(ped, 2);
		SET_PED_ALERTNESS(ped, 100);
		SET_PED_ACCURACY(ped, 100);
		SET_PED_CAN_SWITCH_WEAPON(ped, 1);
		SET_PED_SHOOT_RATE(ped, 200);
		SET_PED_KEEP_TASK(ped, true);
		TASK_COMBAT_HATED_TARGETS_AROUND_PED(ped, 5000.f, 0);
		GIVE_WEAPON_TO_PED(ped, randomWeapon, 100, true, true);
		SET_PED_KEEP_TASK(ped, true);
		SET_PED_GENERATES_DEAD_BODY_EVENTS(ped, true);

		recruitedPeds.emplace_back(ped);
	}
}

static void OnStart()
{
	recruitedPeds.clear();
}

static void OnStop()
{
	for (auto const ped : recruitedPeds)
		if (DOES_ENTITY_EXIST(ped))
			REMOVE_PED_FROM_GROUP(ped);
	recruitedPeds.clear();
}

static void OnTick()
{
	RecruitPed();
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
    {
        .Name = "Recruit Anyone", 
        .Id = "peds_recruit_anyone", 
        .IsTimed = true
    }
);
