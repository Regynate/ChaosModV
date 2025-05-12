#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void DeleteEntity(Entity entity)
{
	if (DOES_ENTITY_EXIST(entity))
	{
		if (!IS_ENTITY_A_MISSION_ENTITY(entity))
			SET_ENTITY_AS_MISSION_ENTITY(entity, false, false);
		DELETE_ENTITY(&entity);
	}
}

struct ChopPed
{
	Ped originalPed;
	Ped attachedChop;
};

CHAOS_VAR std::vector<ChopPed> affectedPeds;

static int GetPedSeatIndex(Ped ped)
{
	auto const vehicle = GET_VEHICLE_PED_IS_IN(ped, false);
	int maxSeats       = GET_VEHICLE_MODEL_NUMBER_OF_SEATS(GET_ENTITY_MODEL(vehicle));
	for (int i = -1; i < maxSeats; i++)
		if (!IS_VEHICLE_SEAT_FREE(vehicle, i, false) && GET_PED_IN_VEHICLE_SEAT(vehicle, i, 0) == ped)
			return i;
	return -2;
}

static void AttachChopToPed()
{
	int counter = 5;

	for (auto const ped : GetAllPeds())
	{
		auto const chopModel = GET_HASH_KEY("A_C_Chop");
		if (!DOES_ENTITY_EXIST(ped) || IS_PED_DEAD_OR_DYING(ped, true) || IS_PED_A_PLAYER(ped) || !IS_PED_HUMAN(ped))
			continue;

		auto const player       = PLAYER_PED_ID();
		auto const playerCoords = GET_ENTITY_COORDS(player, false);
		auto const pedCoords    = GET_ENTITY_COORDS(ped, false);
		auto const distance = GET_DISTANCE_BETWEEN_COORDS(playerCoords.x, playerCoords.y, playerCoords.z, pedCoords.x,
		                                                  pedCoords.y, pedCoords.z, false);

		if (distance > 100.0f)
			continue;

		if (std::any_of(affectedPeds.begin(), affectedPeds.end(),
		                [&](const ChopPed &entry) { return entry.originalPed == ped; }))
			continue;

		auto const heading   = GET_ENTITY_HEADING(ped);
		auto const inVehicle = IS_PED_IN_ANY_VEHICLE(ped, false);
		auto const vehicle   = GET_VEHICLE_PED_IS_IN(ped, false);
		int seat             = -2;

		if (inVehicle)
		{
			SET_ENTITY_AS_MISSION_ENTITY(vehicle, false, false);
			seat = GetPedSeatIndex(ped);
		}

		SET_ENTITY_COORDS(ped, pedCoords.x, pedCoords.y, pedCoords.z + 4.0f, false, false, false, false);

		auto const chop = CreatePoolPed(28, chopModel, pedCoords.x, pedCoords.y,
		                                inVehicle ? pedCoords.z + 2.0 : pedCoords.z, heading);

		if (inVehicle)
		{

			SET_PED_INTO_VEHICLE(chop, vehicle, seat);
			SET_PED_COMBAT_ATTRIBUTES(chop, 3, false);
		}

		SET_ENTITY_VISIBLE(ped, false, false);
		SET_ENTITY_COLLISION(ped, false, false);

		affectedPeds.emplace_back(ChopPed { ped, chop });

		if (--counter <= 0)
		{
			WAIT(0);
			counter = 5;
		}
	}
}

static void UpdateCoords()
{
	for (auto const &entry : affectedPeds)
	{
		auto const ped  = entry.originalPed;
		auto const chop = entry.attachedChop;

		if (DOES_ENTITY_EXIST(ped) && DOES_ENTITY_EXIST(chop))
		{
			auto const chopCoords = GET_ENTITY_COORDS(chop, false);
			SET_ENTITY_COORDS(ped, chopCoords.x, chopCoords.y, chopCoords.z + 1.f, true, false, false, false);
		}
	}
}

static void DetachChop(Ped ped, Ped chop)
{
	if (DOES_ENTITY_EXIST(ped))
	{
		SET_ENTITY_AS_MISSION_ENTITY(ped, false, false);
		SET_ENTITY_VISIBLE(ped, true, false);
		SET_ENTITY_COLLISION(ped, true, true);

		if (DOES_ENTITY_EXIST(chop))
		{
			auto const chopCoords = GET_ENTITY_COORDS(chop, false);

			if (IS_PED_IN_ANY_VEHICLE(chop, false))
			{
				const auto seat       = GetPedSeatIndex(chop);
				
				SET_ENTITY_COORDS(chop, chopCoords.x, chopCoords.y, chopCoords.z + 4.0f, true, false, false, false);
				SET_PED_INTO_VEHICLE(ped, GET_VEHICLE_PED_IS_IN(chop, false), seat);
			}
			else
			{
				SET_ENTITY_COORDS(ped, chopCoords.x, chopCoords.y, chopCoords.z, true, false, false, false);
			}
		}
	}
	if (DOES_ENTITY_EXIST(chop))
		DeleteEntity(chop);
}

static void RestorePeds()
{
	for (auto &entry : affectedPeds)
	{
		auto const ped  = entry.originalPed;
		auto const chop = entry.attachedChop;

		DetachChop(ped, chop);
	}
	affectedPeds.clear();
}

static void CheckChopDeaths()
{
	for (auto it = affectedPeds.begin(); it != affectedPeds.end();)
	{
		if (DOES_ENTITY_EXIST(it->attachedChop) && IS_PED_DEAD_OR_DYING(it->attachedChop, true))
		{
			if (DOES_ENTITY_EXIST(it->originalPed))
				SET_ENTITY_HEALTH(it->originalPed, 0, 0);
			DeleteEntity(it->attachedChop);
			it = affectedPeds.erase(it);
		}
		else
		{
			++it;
		}
	}
}

static void CheckPedsDistance()
{
	auto const player       = PLAYER_PED_ID();
	auto const playerCoords = GET_ENTITY_COORDS(player, false);

	for (auto it = affectedPeds.begin(); it != affectedPeds.end();)
	{
		auto const ped  = it->originalPed;
		auto const chop = it->attachedChop;

		if (!DOES_ENTITY_EXIST(ped) || !DOES_ENTITY_EXIST(chop))
		{
			it = affectedPeds.erase(it);
			continue;
		}

		auto const pedCoords = GET_ENTITY_COORDS(ped, false);
		auto const distance  = GET_DISTANCE_BETWEEN_COORDS(playerCoords.x, playerCoords.y, playerCoords.z, pedCoords.x,
		                                                   pedCoords.y, pedCoords.z, false);

		if (distance > 100.0f)
		{
			DetachChop(ped, chop);
			it = affectedPeds.erase(it);
		}
		else
		{
			++it;
		}
	}
}

static void OnStop()
{
	RestorePeds();
}

static void OnTick()
{
	AttachChopToPed();
	UpdateCoords();
	CheckChopDeaths();
	//CheckPedsDistance();
}

// clang-format off
REGISTER_EFFECT(nullptr, OnStop, OnTick, 
	{ 
		.Name = "Chopception", 
		.Id = "peds_chopception", 
		.IsTimed = true 
	}
);