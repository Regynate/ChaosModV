/*
	Effect by Slothersbee
*/

#include <stdafx.h>
#include <math.h>  

#define PI 3.14159265

static void OnStart()
{
	Ped playerPed = PLAYER_PED_ID();

	static const Hash model = GET_HASH_KEY("ig_tonya");

	static const Hash playerGroup = GET_HASH_KEY("PLAYER");

	Hash relationshipGroup;
	ADD_RELATIONSHIP_GROUP("_HOSTILE_TONYA", &relationshipGroup);
	SET_RELATIONSHIP_BETWEEN_GROUPS(5, relationshipGroup, playerGroup);

	Vector3 playerPos = GET_ENTITY_COORDS(playerPed, false);

	float heading = GET_ENTITY_HEADING(IS_PED_IN_ANY_VEHICLE(playerPed, false) ? GET_VEHICLE_PED_IS_IN(playerPed, false) : playerPed);

	float x_pos = sin((360 - heading) * PI / 180) * 10;
	float y_pos = cos((360 - heading) * PI / 180) * 10;

	Vehicle veh = CreatePoolVehicle(GET_HASH_KEY("towtruck"), playerPos.x - x_pos, playerPos.y - y_pos, playerPos.z, heading);
	SET_VEHICLE_ENGINE_ON(veh, true, true, false);

	Vector3 vel = GET_ENTITY_VELOCITY(playerPed);
	SET_ENTITY_VELOCITY(veh, vel.x, vel.y, vel.z);

	Ped ton = CreatePoolPedInsideVehicle(veh, 4, model, -1);

	SET_PED_RELATIONSHIP_GROUP_HASH(ton, relationshipGroup);

	TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ton, true);

	SET_PED_HEARING_RANGE(ton, 9999.f);
	SET_PED_CONFIG_FLAG(ton, 281, true);

	SET_PED_COMBAT_ATTRIBUTES(ton, 5, true);
	SET_PED_COMBAT_ATTRIBUTES(ton, 46, true);

	SET_PED_SUFFERS_CRITICAL_HITS(ton, false);

	GIVE_WEAPON_TO_PED(ton, GET_HASH_KEY("weapon_bat"), 9999, true, true);
	GIVE_WEAPON_TO_PED(ton, GET_HASH_KEY("weapon_stungun"), 9999, true, true);
	SET_PED_ACCURACY(ton, 100);
	TASK_COMBAT_PED(ton, playerPed, 0, 16);
}

static RegisterEffect registerEffect(EFFECT_SPAWN_TOWING_TONYA, OnStart, EffectInfo
	{
		.Name = "Spawn Towing Tonya",
		.Id = "peds_towingtonya",
		.EEffectGroupType = EEffectGroupType::SpawnEnemy
	}
);