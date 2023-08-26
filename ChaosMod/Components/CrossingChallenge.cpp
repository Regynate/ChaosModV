#include <stdafx.h>

#include "Main.h"
#include "EffectDispatcher.h"
#include "CrossingChallenge.h"

void CrossingChallenge::SetStartParams()
{
	Ped player = PLAYER_PED_ID();

	if (m_hStartVehicleHash)
	{
		LoadModel(m_hStartVehicleHash);
		Vehicle vehicle = CREATE_VEHICLE(m_hStartVehicleHash, m_vStartLocation.x, m_vStartLocation.y,
		                                 m_vStartLocation.z, m_fStartHeading, true, false, false);
		SET_PED_INTO_VEHICLE(player, vehicle, -1);
	}

	SET_ENTITY_HEADING(player, m_fStartHeading);
	SET_GAMEPLAY_CAM_RELATIVE_HEADING(m_fStartCameraHeading);
	_SET_WEATHER_TYPE_TRANSITION(m_hStartWeatherType1, m_hStartWeatherType2, m_fStartWeatherPercent);
	SET_CLOCK_TIME(m_iClockHours, m_iClockMinutes, m_iClockSeconds);
	REMOVE_ALL_PED_WEAPONS(player, false);
	if (m_hStartWeapon)
	{
		GIVE_WEAPON_TO_PED(player, m_hStartWeapon, m_iStartWeaponAmmo, false, true);
	}
}

void CrossingChallenge::ControlRespawn()
{
	Ped player = PLAYER_PED_ID();
	IGNORE_NEXT_RESTART(true);
	PAUSE_DEATH_ARREST_RESTART(true);
	if (IS_ENTITY_DEAD(player, false))
	{
		Vehicle vehicle = 0;

		if (IS_PED_IN_ANY_VEHICLE(player, false))
		{
			SET_PED_TO_RAGDOLL(player, 2000, 2000, 0, true, true, false);
			vehicle = GET_VEHICLE_PED_IS_IN(player, false);
		}

		while (!IS_SCREEN_FADED_OUT())
		{
			GetComponent<EffectDispatcher>()->DrawEffectTexts();
			WAIT(0);
		}
		TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("respawn_controller");
		SET_TIME_SCALE(1.f);
		ANIMPOSTFX_STOP_ALL();
		NETWORK_REQUEST_CONTROL_OF_ENTITY(player);
		NETWORK_RESURRECT_LOCAL_PLAYER(m_vStartLocation.x, m_vStartLocation.y, m_vStartLocation.z, m_fStartHeading,
		                               false, false, false);
		WAIT(2000);
		if (vehicle)
		{
			SET_ENTITY_AS_MISSION_ENTITY(vehicle, true, true);
			DELETE_ENTITY(&vehicle);
		}

		GetComponent<EffectDispatcher>()->Reset();
		ClearEntityPool();

		SetStartParams();

		DO_SCREEN_FADE_IN(2500);
		FORCE_GAME_STATE_PLAYING();
		RESET_PLAYER_ARREST_STATE(player);
		DISPLAY_HUD(true);
		FREEZE_ENTITY_POSITION(player, false);
	}
}

bool LoadScaleform(int &handle, bool &loading, const std::string &name)
{
	if (!handle || !loading && !HAS_SCALEFORM_MOVIE_LOADED(handle))
	{
		handle = REQUEST_SCALEFORM_MOVIE(name.c_str());
		loading = true;
	}
	
	return HAS_SCALEFORM_MOVIE_LOADED(handle);
}

bool CrossingChallenge::CheckEndReached()
{
	Vector3 playerPos = GET_ENTITY_COORDS(PLAYER_PED_ID(), false);
	return GET_DISTANCE_BETWEEN_COORDS(playerPos.x, playerPos.y, 0, m_vEndLocation.x, m_vEndLocation.y, 0, false) < m_fEndRadius;
}

void CrossingChallenge::ShowPassedScaleform()
{
	if (!LoadScaleform(m_hPassedScaleformHandle, m_bPassedScaleformLoading, "MP_BIG_MESSAGE_FREEMODE")
	    || !IS_MISSION_COMPLETE_READY_FOR_UI())
	{
		return;
	}
	else if (m_bPassedScaleformLoading)
	{
		BEGIN_SCALEFORM_MOVIE_METHOD(m_hPassedScaleformHandle, "SHOW_SHARD_RANKUP_MP_MESSAGE");
		BEGIN_TEXT_COMMAND_SCALEFORM_STRING("STRING");
		ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME("YES!!!");
		END_TEXT_COMMAND_SCALEFORM_STRING();
		BEGIN_TEXT_COMMAND_SCALEFORM_STRING("STRING");
		ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME("You win! Congratulations!!!!");
		END_TEXT_COMMAND_SCALEFORM_STRING();
		SCALEFORM_MOVIE_METHOD_ADD_PARAM_INT(14); // HUD_COLOUR_YELLOWDARK
		END_SCALEFORM_MOVIE_METHOD();

		m_bPassedScaleformLoading = false;
	}

	if (!m_bPassedTransitionStarted && GET_GAME_TIMER() - m_iPassedScaleformTick > 5000)
	{
		CALL_SCALEFORM_MOVIE_METHOD(m_hPassedScaleformHandle, "TRANSITION_OUT");
		m_bPassedTransitionStarted = true;
	}

	DRAW_SCALEFORM_MOVIE_FULLSCREEN(m_hPassedScaleformHandle, 255, 255, 255, 255, 0);

	if (GET_GAME_TIMER() - m_iPassedScaleformTick > 6000)
	{
		m_iPassedState = 2;
	}
}

void CrossingChallenge::ControlPassed()
{
	switch (m_iPassedState)
	{
	case 0:
		if (CheckEndReached())
		{
			m_iPassedState = 1;
			if (ComponentExists<EffectDispatcher>())
			{
				GetComponent<EffectDispatcher>()->SetPaused(true);
			}
			m_iPassedScaleformTick = GET_GAME_TIMER();
			PLAY_MISSION_COMPLETE_AUDIO("MICHAEL_SMALL_01");
		}
		else
		{
			break;
		}
		[[fallthrough]];
	case 1:
		ShowPassedScaleform();
		break;
	case 2:
		Main::Stop();
		break;
	}
}

void CrossingChallenge::SaveConfig()
{
	LOG("Saving");

	m_fConfigFile.SetValue<bool>("StartEnabled", 		  m_bStartEnabled       );
	m_fConfigFile.SetValue<float>("StartLocationX", 	  m_vStartLocation.x      );
	m_fConfigFile.SetValue<float>("StartLocationY", m_vStartLocation.y);
	m_fConfigFile.SetValue<float>("StartLocationZ", m_vStartLocation.z);
	m_fConfigFile.SetValue<Hash>("StartVehicle",		  m_hStartVehicleHash   );
	m_fConfigFile.SetValue<float>("StartHeading", 		  m_fStartHeading       );
	m_fConfigFile.SetValue<float>("StartCameraHeading",  m_fStartCameraHeading );
	m_fConfigFile.SetValue<Hash>("StartWeather1",		  m_hStartWeatherType1  );
	m_fConfigFile.SetValue<Hash>("StartWeather2",		  m_hStartWeatherType2  );
	m_fConfigFile.SetValue<float>("StartWeatherPercent", m_fStartWeatherPercent);
	m_fConfigFile.SetValue<int>("StartHours",			  m_iClockHours         );
	m_fConfigFile.SetValue<int>("StartMinutes",		  m_iClockMinutes       );
	m_fConfigFile.SetValue<int>("StartSeconds",		  m_iClockSeconds       );
	m_fConfigFile.SetValue<Hash>("StartWeapon",		  m_hStartWeapon        );
	if (m_hStartWeapon)
	{
		m_fConfigFile.SetValue<int>("StartAmmo", m_iStartWeaponAmmo);
	}

	m_fConfigFile.SetValue<bool>("EndEnabled", m_bEndEnabled);
	m_fConfigFile.SetValue<float>("EndLocationX", m_vEndLocation.x);
	m_fConfigFile.SetValue<float>("EndLocationY", m_vEndLocation.y);
	m_fConfigFile.SetValue<float>("EndLocationZ", m_vEndLocation.z);
	m_fConfigFile.SetValue<float>("EndRadius", m_fEndRadius);

	m_fConfigFile.WriteFile();
}

void CrossingChallenge::CaptureStart()
{
	LOG("Capture Start");

	Ped player       = PLAYER_PED_ID();

	m_bStartEnabled  = true;
	m_vStartLocation = GET_ENTITY_COORDS(player, false);
	if (IS_PED_IN_ANY_VEHICLE(player, false))
	{
		m_hStartVehicleHash = GET_ENTITY_MODEL(GET_VEHICLE_PED_IS_IN(player, false));
	}
	m_fStartHeading = GET_ENTITY_HEADING(player);
	m_fStartCameraHeading = GET_GAMEPLAY_CAM_RELATIVE_HEADING();
	_GET_WEATHER_TYPE_TRANSITION(&m_hStartWeatherType1, &m_hStartWeatherType2, &m_fStartWeatherPercent);
	m_iClockHours   = GET_CLOCK_HOURS();
	m_iClockMinutes = GET_CLOCK_MINUTES();
	m_iClockSeconds = GET_CLOCK_SECONDS();
	m_hStartWeapon  = GET_SELECTED_PED_WEAPON(player);
	if (m_hStartWeapon)
	{
		GET_AMMO_IN_CLIP(player, m_hStartWeapon, &m_iStartWeaponAmmo);
	}

	SaveConfig();
}

void CrossingChallenge::CaptureEnd()
{
	m_bEndEnabled  = true;
	m_vEndLocation = GET_ENTITY_COORDS(PLAYER_PED_ID(), false);
	m_fEndRadius   = 10.f;

	SaveConfig();
}

CrossingChallenge::CrossingChallenge()
{
	m_bStartEnabled = m_fConfigFile.ReadValue<bool>("StartEnabled", false);
	if (m_bStartEnabled)
	{
		m_vStartLocation       = Vector3(m_fConfigFile.ReadValue<float>("StartLocationX", 0.f),
		                                 m_fConfigFile.ReadValue<float>("StartLocationY", 0.f),
		                                 m_fConfigFile.ReadValue<float>("StartLocationZ", 0.f));
		m_hStartVehicleHash    = m_fConfigFile.ReadValue<Hash>("StartVehicle", 0);
		m_fStartHeading        = m_fConfigFile.ReadValue<float>("StartHeading", 0.f);
		m_fStartCameraHeading  = m_fConfigFile.ReadValue<float>("StartCameraHeading", 0.f);
		m_hStartWeatherType1   = m_fConfigFile.ReadValue<Hash>("StartWeather1", 0);
		m_hStartWeatherType2   = m_fConfigFile.ReadValue<Hash>("StartWeather2", 0);
		m_fStartWeatherPercent = m_fConfigFile.ReadValue<float>("StartWeatherPercent", 0.f);
		m_iClockHours          = m_fConfigFile.ReadValue<int>("StartHours", 0);
		m_iClockMinutes        = m_fConfigFile.ReadValue<int>("StartMinutes", 0);
		m_iClockSeconds        = m_fConfigFile.ReadValue<int>("StartSeconds", 0);
		m_hStartWeapon         = m_fConfigFile.ReadValue<Hash>("StartWeapon", 0);
		if (m_hStartWeapon)
		{
			m_iStartWeaponAmmo = m_fConfigFile.ReadValue<int>("StartAmmo", 0);
		}
	}

	m_bEndEnabled  = m_fConfigFile.ReadValue<bool>("EndEnabled", false);
	if (m_bEndEnabled)
	{
		m_vEndLocation = Vector3(m_fConfigFile.ReadValue<float>("EndLocationX", 0.f),
		                         m_fConfigFile.ReadValue<float>("EndLocationY", 0.f),
		                         m_fConfigFile.ReadValue<float>("EndLocationZ", 0.f));
		m_fEndRadius   = m_fConfigFile.ReadValue<float>("EndRadius", 0.f);
	}
}

void DrawTriangle(Vector3 A, Vector3 B, Vector3 C, int r, int g, int b, int a)
{
	DRAW_POLY(A.x, A.y, A.z, B.x, B.y, B.z, C.x, C.y, C.z, r, g, b, a);
	DRAW_POLY(B.x, B.y, B.z, A.x, A.y, A.z, C.x, C.y, C.z, r, g, b, a);
}

void DrawCylinder(Vector3 center, float radius, float height, int r, int g, int b, int a, int polygons = 100)
{
	Vector3 uc = center + Vector3(0, 0, height / 2);
	Vector3 dc = center - Vector3(0, 0, height / 2);

	for (int i = 0; i < polygons; i++)
	{
		Vector3 delta1 = Vector3(SIN(360.f * i / polygons), COS(360.f * i / polygons), 0);
		Vector3 delta2 = Vector3(SIN(360.f * (i + 1) / polygons), COS(360.f * (i + 1) / polygons), 0);

		Vector3 u1 = uc + delta1 * radius, u2 = uc + delta2 * radius, d1 = dc + delta1 * radius, d2 = dc + delta2 * radius;

		DrawTriangle(u1, u2, uc, r, g, b, a);
		DrawTriangle(d1, d2, dc, r, g, b, a);
		DrawTriangle(d2, d1, u1, r, g, b, a);
		DrawTriangle(d2, u2, u1, r, g, b, a);
	}
}

bool CrossingChallenge::IsEndValid()
{
	return !m_bEndEnabled || !m_bStartEnabled
	    || GET_DISTANCE_BETWEEN_COORDS(m_vStartLocation.x, m_vStartLocation.y, 0, 
									   m_vEndLocation.x, m_vEndLocation.y, 0, false) > m_fEndRadius * 1.5f;
}

void CrossingChallenge::ShowEndCylinder()
{
	if (IsEndValid())
	{
		DrawCylinder(m_vEndLocation, m_fEndRadius, 50.f, 255, 255, 0, 120);
	}
	else
	{
		DrawCylinder(m_vEndLocation, m_fEndRadius, 50.f, 255, 0, 0, 120);
	}
}

void CrossingChallenge::IncreaseEndRadius()
{
	m_fEndRadius += 1.f;
}

void CrossingChallenge::DecreaseEndRadius()
{
	if (m_fEndRadius >= 2.f)
	{
		m_fEndRadius -= 1.f;
	}
}

void CrossingChallenge::ShowHint(const std::string &text)
{
	BEGIN_TEXT_COMMAND_DISPLAY_HELP("STRING");
	ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text.c_str());
	END_TEXT_COMMAND_DISPLAY_HELP(0, 1, 1, -1);
	m_iHelpMessageTick = GET_GAME_TIMER();
}

void AddButton(int scaleform, int i, const std::string &control, const std::string &text)
{
	BEGIN_SCALEFORM_MOVIE_METHOD(scaleform, "SET_DATA_SLOT");
	SCALEFORM_MOVIE_METHOD_ADD_PARAM_INT(i);
	SCALEFORM_MOVIE_METHOD_ADD_PARAM_PLAYER_NAME_STRING(control.c_str());
	BEGIN_TEXT_COMMAND_SCALEFORM_STRING("STRING");
	ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(text.c_str());
	END_TEXT_COMMAND_SCALEFORM_STRING();
	END_SCALEFORM_MOVIE_METHOD();
}

void CrossingChallenge::ShowHelpButtons()
{
	if (!LoadScaleform(m_hButtonsScaleformHandle, m_bButtonsScaleformLoading, "INSTRUCTIONAL_BUTTONS"))
	{
		return;
	}
	else
	{
		m_bButtonsScaleformLoading = false;
	}

	CALL_SCALEFORM_MOVIE_METHOD(m_hButtonsScaleformHandle, "CLEAR_ALL");
	CALL_SCALEFORM_MOVIE_METHOD_WITH_NUMBER(m_hButtonsScaleformHandle, "SET_CLEAR_SPACE", 200, -1.0, -1.0, -1.0, -1.0);

	int i = 0;

	if (IsEndValid())
	{
		if (m_bWaitingConfirm)
		{
			AddButton(m_hButtonsScaleformHandle, i++, "b_1003", "Start?");
		}
		else
		{
			AddButton(m_hButtonsScaleformHandle, i++, "b_2000", "Confirm and start");
		}
	}
	if (m_bEndEnabled)
	{
		AddButton(m_hButtonsScaleformHandle, i++, "b_1009", "Increase finish radius");
		AddButton(m_hButtonsScaleformHandle, i++, "b_1010", "Decrease finish radius");
	}
	AddButton(m_hButtonsScaleformHandle, i++, "t_Q", "Capture finish point");
	AddButton(m_hButtonsScaleformHandle, i++, "t_E", "Capture starting point");

	CALL_SCALEFORM_MOVIE_METHOD(m_hButtonsScaleformHandle, "DRAW_INSTRUCTIONAL_BUTTONS");

	CALL_SCALEFORM_MOVIE_METHOD_WITH_NUMBER(m_hButtonsScaleformHandle, "SET_BACKGROUND_COLOUR", 0, 0, 0, 80, -1.0);


	DRAW_SCALEFORM_MOVIE_FULLSCREEN(m_hButtonsScaleformHandle, 255, 255, 255, 255, 0);
}

void HandleBlip(Blip& blip, Vector3 coords, int color, const std::string& name)
{
	if (!blip || !DOES_BLIP_EXIST(blip))
	{
		blip = ADD_BLIP_FOR_COORD(coords.x, coords.y, coords.z);
		SET_BLIP_COLOUR(blip, color);
		SET_BLIP_DISPLAY(blip, 2);
		BEGIN_TEXT_COMMAND_SET_BLIP_NAME("STRING");
		ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME(name.c_str());
		END_TEXT_COMMAND_SET_BLIP_NAME(blip);
	}
	else
	{
		SET_BLIP_COORDS(blip, coords.x, coords.y, coords.z);
	}
}

void CrossingChallenge::ShowBlips()
{
	if (m_bStartEnabled)
	{
		HandleBlip(m_bStartBlip, m_vStartLocation, 2, "Start");
	}
	if (m_bEndEnabled)
	{
		HandleBlip(m_bEndBlip, m_vEndLocation, 5, "Finish");
	}
}

void CrossingChallenge::OnRun()
{
	if (m_iHelpMessageTick != -1 && GET_GAME_TIMER() - m_iHelpMessageTick > 5000)
	{
		CLEAR_ALL_HELP_MESSAGES();
		m_iHelpMessageTick = -1;
	}

	if (m_bCaptureStartFlag)
	{
		CaptureStart();
		ShowHint("Starting point captured");
		m_bCaptureStartFlag = false;
	}
	if (m_bCaptureEndFlag)
	{
		CaptureEnd();
		ShowHint("Finish point captured");
		m_bCaptureEndFlag = false;
	}

	if (m_bStartedState == 0)
	{
		SET_ENTITY_INVINCIBLE(PLAYER_PED_ID(), true);
		ShowHelpButtons();
		ShowBlips();

		if (m_bEndEnabled)
		{
			ShowEndCylinder();
		}
	}
	else if (m_bStartedState == 1)
	{
		if (m_bStartEnabled)
		{
			DO_SCREEN_FADE_OUT(1000);
			NEW_LOAD_SCENE_START_SPHERE(m_vStartLocation.x, m_vStartLocation.y, m_vStartLocation.z, 200.f, 0);
			WAIT(1000);
			SET_ENTITY_COORDS(PLAYER_PED_ID(), m_vStartLocation.x, m_vStartLocation.y, m_vStartLocation.z, 1, 0, 0, 1);
			int startTick = GET_GAME_TIMER();
			while (!IS_NEW_LOAD_SCENE_LOADED() && GET_GAME_TIMER() - startTick < 10000)
			{
				SET_TIME_SCALE(1.f);
				WAIT(0);
			}

			SetStartParams();
			SET_BLIP_DISPLAY(m_bStartBlip, 0);

			DO_SCREEN_FADE_IN(1000);
		}

		if (m_bEndEnabled)
		{
			SET_BLIP_ROUTE(m_bEndBlip, true);
		}

		SET_ENTITY_INVINCIBLE(PLAYER_PED_ID(), false);
		if (ComponentExists<EffectDispatcher>())
		{
			GetComponent<EffectDispatcher>()->Reset();
		}
		m_bStartedState = 2;
	}
	else
	{
		if (m_bStartEnabled)
		{
			ControlRespawn();
		}
		else
		{
			IGNORE_NEXT_RESTART(false);
			PAUSE_DEATH_ARREST_RESTART(false);
		}

		if (m_bEndEnabled)
		{
			ControlPassed();
		}
	}
}

void CrossingChallenge::OnModPauseCleanup()
{
	LOG("PauseCleanup");

	m_bStartedState           = 0;
	m_hButtonsScaleformHandle = 0;
	m_bButtonsScaleformLoading    = false;
	if (DOES_BLIP_EXIST(m_bStartBlip))
	{
		REMOVE_BLIP(&m_bStartBlip);
	}
	if (DOES_BLIP_EXIST(m_bEndBlip))
	{
		REMOVE_BLIP(&m_bEndBlip);
	}

	if (m_bStartedState != 2)
	{
		SET_ENTITY_INVINCIBLE(PLAYER_PED_ID(), false);
	}

	if (m_iHelpMessageTick != -1)
	{
		CLEAR_ALL_HELP_MESSAGES();
		m_iHelpMessageTick = -1;
	}
}

void CrossingChallenge::HandleInput(DWORD ulKey, bool bOnRepeat)
{
	if (m_bStartedState == 0)
	{
		if (!bOnRepeat)
		{
			if (ulKey == 0x45) // E
			{
				m_bCaptureStartFlag = true;
			}
			else if (ulKey == 0x51) // Q
			{
				m_bCaptureEndFlag = true;
			}
			else if (ulKey == 0x20) // Space
			{
				m_bWaitingConfirm = true;
			}
			else if (ulKey == 0x21) // PgUp
			{
				IncreaseEndRadius();
			}
			else if (ulKey == 0x22) // PgDn
			{
				DecreaseEndRadius();
			}
			else if (ulKey == 0x0D && m_bWaitingConfirm) // Enter
			{
				m_bWaitingConfirm = false;
				m_bStartedState   = 1;
			}
		}

		if (ulKey != 0x20) // Space
		{
			m_bWaitingConfirm = false;
		}
	}
}