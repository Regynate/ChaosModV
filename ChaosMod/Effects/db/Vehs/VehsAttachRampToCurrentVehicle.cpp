#include <stdafx.h>
#include "Effects/Register/RegisterEffect.h"

Vector3 GetCoordsInFront(const std::int32_t entity, const float distance) {
	auto constexpr PI     = 3.1415f;
    auto const entityCoords = GET_ENTITY_COORDS(entity, false);
    auto const heading = GET_ENTITY_HEADING(entity) * (PI / 180.0f);

    auto const offsetX = distance * std::sin(heading);
    auto const offsetY = distance * std::cos(heading);

    return { entityCoords.x + offsetX, entityCoords.y + offsetY, entityCoords.z };
}

void OnStart() {
    auto const player = PLAYER_PED_ID();
    auto const inVehicle = IS_PED_IN_ANY_VEHICLE(player, false);
    if (!inVehicle) {
        return;
    }

    auto const rampModel = GET_HASH_KEY("prop_mp_ramp_03");
    LoadModel(rampModel);

    auto const vehicle = GET_VEHICLE_PED_IS_IN(player, false);
    auto const heading = GET_ENTITY_HEADING(vehicle);

    auto const inFrontOfVehicle = GetCoordsInFront(vehicle, 5.f);
	auto const ramp = CreatePoolProp(rampModel, inFrontOfVehicle.x, inFrontOfVehicle.y, inFrontOfVehicle.z, false);
	auto const ramp2 = CreatePoolProp(rampModel, inFrontOfVehicle.x, inFrontOfVehicle.y, inFrontOfVehicle.z, false);

    SET_ENTITY_ALPHA(ramp, 150, false);
    SET_ENTITY_ALPHA(ramp2, 150, false);

    ATTACH_ENTITY_TO_ENTITY(ramp, vehicle, 0, 0, 5, 0, 0, 0, 180, false, false, true, false, 0, true);
    ATTACH_ENTITY_TO_ENTITY(ramp2, vehicle, 0, 0, 5, -1, 0, 0, 180, false, false, true, false, 0, true);

    SET_ENTITY_DYNAMIC(ramp, true);
    SET_ENTITY_COLLISION(ramp, true, true);
    SET_ENTITY_DYNAMIC(ramp2, true);
    SET_ENTITY_COLLISION(ramp2, true, true);
}

void OnStop() {}

void OnTick() {}

REGISTER_EFFECT(OnStart, nullptr, nullptr, 
    {
        .Name = "Attach Ramp to Current Vehicle",
        .Id = "vehs_attach_ramp_to_current_vehicle",
        .IsTimed = false
    }
);
