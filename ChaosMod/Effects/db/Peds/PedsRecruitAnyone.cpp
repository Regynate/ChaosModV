#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"
#include "Util/HelpText.h"

CHAOS_VAR std::vector<Entity> recruitedPeds;

CHAOS_VAR const std::array<Hash, 3> weaponList = { GET_HASH_KEY("weapon_pistol"), GET_HASH_KEY("weapon_smg"),
	                                               GET_HASH_KEY("weapon_assaultrifle") };

static Ped GetAimedPed()
{
	auto const player = PLAYER_PED_ID();
	Ped aimedEntity;
	auto const isAimingAtEntity = GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(PLAYER_ID(), &aimedEntity);
	auto const isPed            = IS_ENTITY_A_PED(aimedEntity);
	auto const isNotInVehicle   = !IS_PED_IN_ANY_VEHICLE(aimedEntity, true);

	if (isAimingAtEntity && isPed && isNotInVehicle)
		return aimedEntity;
	return 0;
}

static void RecruitPed()
{
	if (IS_DISABLED_CONTROL_JUST_PRESSED(0, 46)) // INPUT_TALK
	{
		auto const ped              = GetAimedPed();
		auto const alreadyRecruited = std::find(recruitedPeds.begin(), recruitedPeds.end(), ped) != recruitedPeds.end();
		if (!ped || alreadyRecruited)
			return;

		auto const max          = weaponList.size();
		auto const randomIndex  = GET_RANDOM_INT_IN_RANGE(0, max);
		auto const randomWeapon = weaponList[randomIndex];

		auto const group        = GET_PED_GROUP_INDEX(PLAYER_PED_ID());
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
	DisplayHelpText("Recruit: Aim at a ped and press 'E'");
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
