/*
    Effect by Lucas7yoshi, modified
*/

#include <stdafx.h>

#include "Effects/Register/RegisterEffect.h"

static void OnStart()
{
	REQUEST_ADDITIONAL_TEXT("CREDIT", 0);
	while (!HAS_ADDITIONAL_TEXT_LOADED(0))
		WAIT(0);

	PLAY_END_CREDITS_MUSIC(true);
	SET_CREDITS_ACTIVE(true);
	SET_MOBILE_PHONE_RADIO_STATE(true);

	int song = g_Random.GetRandomInt(0, 2);
	if (song == 0)
		SET_CUSTOM_RADIO_TRACK_LIST("RADIO_16_SILVERLAKE", "END_CREDITS_SAVE_MICHAEL_TREVOR", 1);
	else if (song == 1)
		SET_CUSTOM_RADIO_TRACK_LIST("RADIO_16_SILVERLAKE", "END_CREDITS_KILL_MICHAEL", 1);
	else
		SET_CUSTOM_RADIO_TRACK_LIST("RADIO_16_SILVERLAKE", "END_CREDITS_KILL_TREVOR", 1);
}

static void OnStop()
{
	SET_ENTITY_INVINCIBLE(PLAYER_PED_ID(), false);

	SET_CREDITS_ACTIVE(false);
	PLAY_END_CREDITS_MUSIC(false);
	SET_MOBILE_PHONE_RADIO_STATE(false);

	SET_USER_RADIO_CONTROL_ENABLED(true);
}

static void OnTick()
{
	SET_ENTITY_INVINCIBLE(PLAYER_PED_ID(), true);

	SET_RADIO_TO_STATION_NAME("RADIO_16_SILVERLAKE");

	SET_USER_RADIO_CONTROL_ENABLED(false);

	DRAW_RECT(
	    .5f, .5f, 1.f, 1.f, 0, 0, 0,
	    static_cast<int>(std::floor(std::lerp(0, 255, std::min(CurrentEffect::GetEffectCompletionPercentage(), 1.f)))),
	    false);
}

// clang-format off
REGISTER_EFFECT(OnStart, OnStop, OnTick, 
	{
		.Name = "Roll Credits",
		.Id = "misc_credits",
		.IsTimed = true,
		.IsShortDuration = true
	}
);