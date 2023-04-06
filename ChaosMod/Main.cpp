#include <stdafx.h>

#include "Main.h"
#include "Mp3Manager.h"

#include "Effects/EffectConfig.h"

#include "Memory/Hooks/ScriptThreadRunHook.h"
#include "Memory/Hooks/ShaderHook.h"
#include "Memory/Misc.h"
#include "Memory/Shader.h"

#include "Components/DebugMenu.h"
#include "Components/DebugSocket.h"
#include "Components/EffectDispatcher.h"
#include "Components/Failsafe.h"
#include "Components/Shortcuts.h"
#include "Components/SplashTexts.h"
#include "Components/TwitchVoting.h"

#include "Util/File.h"
#include "Util/OptionsManager.h"
#include "Util/PoolSpawner.h"

static Vector3 ms_vSpawnLocation            = Vector3();

static bool ms_bClearAllEffects             = false;
static bool ms_bClearEffectsShortcutEnabled = false;
static bool ms_bToggleModShortcutEnabled    = false;
static bool ms_bDisableMod                  = false;
static bool ms_bEnablePauseTimerShortcut    = false;
static bool ms_bHaveLateHooksRan            = false;
static bool ms_bAntiSoftlockShortcutEnabled = false;
static bool ms_bRunAntiSoftlock             = false;

static bool ms_bWantedLevelFlag             = false;

_NODISCARD static std::array<BYTE, 3> ParseConfigColorString(const std::string &szColorText)
{
	// Format: #ARGB
	std::array<BYTE, 3> rgColors;

	int j = 0;
	for (int i = 3; i < 9; i += 2)
	{
		Util::TryParse<BYTE>(szColorText.substr(i, 2), rgColors[j++], 16);
	}

	return rgColors;
}

static void ParseEffectsFile()
{
	g_dictEnabledEffects.clear();

	EffectConfig::ReadConfig("chaosmod/effects.ini", g_dictEnabledEffects);
}

static void Reset()
{
	// Check if this isn't the first time this is being run
	if (ComponentExists<EffectDispatcher>())
	{
		LOG("Mod has been disabled");
	}

	ClearEntityPool();

	Mp3Manager::ResetCache();

	LuaScripts::Unload();

	for (auto pComponent : g_rgComponents)
	{
		pComponent->OnModPauseCleanup();
	}
}

static void ControlRespawn()
{
	if (!ms_vSpawnLocation.IsDefault())
	{
		Ped player = PLAYER_PED_ID();
		IGNORE_NEXT_RESTART(true);
		PAUSE_DEATH_ARREST_RESTART(true);
		if (IS_ENTITY_DEAD(player, false))
		{
			while (!IS_SCREEN_FADED_OUT())
			{
				WAIT(0);
			}
			TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("respawn_controller");
			SET_TIME_SCALE(1.f);
			ANIMPOSTFX_STOP_ALL();
			NETWORK_REQUEST_CONTROL_OF_ENTITY(player);
			NETWORK_RESURRECT_LOCAL_PLAYER(ms_vSpawnLocation.x, ms_vSpawnLocation.y, ms_vSpawnLocation.z,
			                               GET_ENTITY_HEADING(player), false, false, false);
			WAIT(2000);
			DO_SCREEN_FADE_IN(3500);
			FORCE_GAME_STATE_PLAYING();
			RESET_PLAYER_ARREST_STATE(player);
			DISPLAY_HUD(true);
			FREEZE_ENTITY_POSITION(player, false);

			GetComponent<EffectDispatcher>()->Reset();
			ClearEntityPool();
		}

		if (GET_ENTITY_COORDS(player, false).DistanceTo(ms_vSpawnLocation) <= 500)
		{
			if (!ms_bWantedLevelFlag)
			{
				SET_MAX_WANTED_LEVEL(0);
				CLEAR_PLAYER_WANTED_LEVEL(PLAYER_ID());
				ms_bWantedLevelFlag = true;
			}
		}
		else
		{
			SET_MAX_WANTED_LEVEL(5);
			ms_bWantedLevelFlag = false;
		}
	}
	else
	{
		IGNORE_NEXT_RESTART(false);
		PAUSE_DEATH_ARREST_RESTART(false);
		if (ms_bWantedLevelFlag)
		{
			SET_MAX_WANTED_LEVEL(5);
			ms_bWantedLevelFlag = false;
		}
	}
}

static void Init()
{
	// Attempt to print game build number
	// We're doing it here as the build number isn't available when the mod is attached to the game process
	static auto printedGameBuild = []()
	{
		auto gameBuild = Memory::GetGameBuild();
		if (gameBuild.empty())
		{
			gameBuild = "Unknown";
		}

		LOG("Game Build: " << gameBuild);

		return true;
	}();

	static std::streambuf *c_pOldStreamBuf;
	if (DoesFileExist("chaosmod\\.enableconsole"))
	{
		if (GetConsoleWindow())
		{
			system("cls");
		}
		else
		{
			LOG("Creating log console");

			AllocConsole();

			SetConsoleTitle(L"ChaosModV");
			DeleteMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);

			c_pOldStreamBuf = std::cout.rdbuf();

			g_ConsoleOut    = std::ofstream("CONOUT$");
			std::cout.rdbuf(g_ConsoleOut.rdbuf());

			std::cout.clear();

			HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

			DWORD ulConMode;
			GetConsoleMode(handle, &ulConMode);
			SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ulConMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
	}
	else if (GetConsoleWindow())
	{
		LOG("Destroying log console");

		std::cout.rdbuf(c_pOldStreamBuf);

		g_ConsoleOut.close();

		FreeConsole();
	}

	LOG("Parsing config files");
	ParseEffectsFile();

	g_OptionsManager.Reset();

	ms_bClearEffectsShortcutEnabled =
	    g_OptionsManager.GetConfigValue<bool>("EnableClearEffectsShortcut", OPTION_DEFAULT_SHORTCUT_CLEAR_EFFECTS);
	ms_bToggleModShortcutEnabled =
	    g_OptionsManager.GetConfigValue<bool>("EnableToggleModShortcut", OPTION_DEFAULT_SHORTCUT_TOGGLE_MOD);
	ms_bEnablePauseTimerShortcut =
	    g_OptionsManager.GetConfigValue<bool>("EnablePauseTimerShortcut", OPTION_DEFAULT_SHORTCUT_PAUSE_TIMER);
	ms_bAntiSoftlockShortcutEnabled =
	    g_OptionsManager.GetConfigValue<bool>("EnableAntiSoftlockShortcut", OPTION_DEFAULT_SHORTCUT_ANTI_SOFTLOCK);

	g_bEnableGroupWeighting =
	    g_OptionsManager.GetConfigValue<bool>("EnableGroupWeightingAdjustments", OPTION_DEFAULT_GROUP_WEIGHTING);

	const auto &rgTimerColor = ParseConfigColorString(
	    g_OptionsManager.GetConfigValue<std::string>("EffectTimerColor", OPTION_DEFAULT_BAR_COLOR));
	const auto &rgTextColor = ParseConfigColorString(
	    g_OptionsManager.GetConfigValue<std::string>("EffectTextColor", OPTION_DEFAULT_TEXT_COLOR));
	const auto &rgEffectTimerColor = ParseConfigColorString(
	    g_OptionsManager.GetConfigValue<std::string>("EffectTimedTimerColor", OPTION_DEFAULT_TIMED_COLOR));

	LOG("Running custom scripts");
	LuaScripts::Load();

	g_Random.SetSeed(g_OptionsManager.GetConfigValue<int>("Seed", 0));

	LOG("Initializing effects dispatcher");
	InitComponent<EffectDispatcher>(rgTimerColor, rgTextColor, rgEffectTimerColor);

	InitComponent<DebugMenu>();

	LOG("Initializing shortcuts");
	InitComponent<Shortcuts>();

	LOG("Initializing Twitch voting");
	InitComponent<TwitchVoting>(rgTextColor);

	LOG("Initializing Failsafe");
	InitComponent<Failsafe>();

	LOG("Initializing Splash Texts");
	InitComponent<SplashTexts>();
	GetComponent<SplashTexts>()->ShowInitSplash();

#ifdef WITH_DEBUG_PANEL_SUPPORT
	if (DoesFileExist("chaosmod\\.enabledebugsocket"))
	{
		LOG("Initializing Debug Websocket");
		InitComponent<DebugSocket>();
	}
#endif


	LOG("Completed init!");

	if (ComponentExists<TwitchVoting>() && GetComponent<TwitchVoting>()->IsEnabled() && ComponentExists<SplashTexts>())
	{
		GetComponent<SplashTexts>()->ShowTwitchVotingSplash();
	}
}

static void MainRun()
{
	if (!ms_bHaveLateHooksRan)
	{
		ms_bHaveLateHooksRan = true;

		Memory::RunLateHooks();
	}

	g_MainThread = GetCurrentFiber();

	EffectThreads::ClearThreads();

	Reset();

	ms_bDisableMod = g_OptionsManager.GetConfigValue<bool>("DisableStartup", OPTION_DEFAULT_DISABLE_STARTUP);

	Init();

	bool c_bJustReenabled = false;

	while (true)
	{
		WAIT(0);

		DISABLE_CONTROL_ACTION(0, 0x12, true); // block cutscene skips

		if (!ms_bDisableMod)
		{
			ControlRespawn();
		}
		else if (ms_bWantedLevelFlag)
		{
			SET_MAX_WANTED_LEVEL(5);
			ms_bWantedLevelFlag = false;
		}

		// This will run regardless if mod is disabled
		if (ms_bRunAntiSoftlock)
		{
			ms_bRunAntiSoftlock = false;
			if (IS_SCREEN_FADED_OUT())
			{
				DO_SCREEN_FADE_IN(0);
				SET_ENTITY_HEALTH(PLAYER_PED_ID(), 0, 0);
			}
		}

		if (!EffectThreads::IsAnyThreadRunningOnStart())
		{
			if (ms_bDisableMod && !c_bJustReenabled)
			{
				c_bJustReenabled = true;
				Reset();

				continue;
			}
			else if (c_bJustReenabled)
			{
				if (!ms_bDisableMod)
				{
					c_bJustReenabled = false;

					LOG("Mod has been re-enabled");

					// Restart the main part of the mod completely
					Init();
				}

				continue;
			}

			if (ms_bClearAllEffects)
			{
				ms_bClearAllEffects = false;
				GetComponent<EffectDispatcher>()->Reset();
				ClearEntityPool();
			}
		}
		else if (IS_SCREEN_FADED_OUT())
		{
			// Prevent potential softlock for certain effects
			SET_TIME_SCALE(1.f);
			Hooks::DisableScriptThreadBlock();
			WAIT(100);

			continue;
		}

		for (auto pComponent : g_rgComponents)
		{
			pComponent->OnRun();
		}
	}
}

namespace Main
{
	void OnRun()
	{
		SetUnhandledExceptionFilter(CrashHandler);

		MainRun();
	}

	void OnCleanup()
	{
		LuaScripts::Unload();
	}

	void OnKeyboardInput(DWORD ulKey, WORD usRepeats, BYTE ucScanCode, BOOL bIsExtended, BOOL bIsWithAlt,
	                     BOOL bWasDownBefore, BOOL bIsUpNow)
	{
		static bool c_bIsCtrlPressed  = false;
		static bool c_bIsShiftPressed = false;

		if (ulKey == VK_CONTROL)
		{
			c_bIsCtrlPressed = !bIsUpNow;
		}
		else if (ulKey == VK_SHIFT)
		{
			c_bIsShiftPressed = !bIsUpNow;
		}
		else if (ulKey == 0x4B) // K
		{
			STOP_CUTSCENE(0);
		}
		else if (c_bIsCtrlPressed && !bWasDownBefore)
		{
			if (ulKey == VK_OEM_MINUS && ms_bClearEffectsShortcutEnabled)
			{
				ms_bClearAllEffects = true;

				if (ComponentExists<SplashTexts>())
				{
					GetComponent<SplashTexts>()->ShowClearEffectsSplash();
				}
			}
			else if (ulKey == VK_OEM_PERIOD && ms_bEnablePauseTimerShortcut && ComponentExists<EffectDispatcher>())
			{
				GetComponent<EffectDispatcher>()->m_bPauseTimer = !GetComponent<EffectDispatcher>()->m_bPauseTimer;
			}
			else if (ulKey == VK_OEM_COMMA && ComponentExists<DebugMenu>() && GetComponent<DebugMenu>()->IsEnabled())
			{
				GetComponent<DebugMenu>()->SetVisible(!GetComponent<DebugMenu>()->IsVisible());
			}
			else if (ulKey == 0x4B && ms_bAntiSoftlockShortcutEnabled && c_bIsShiftPressed) // K
			{
				ms_bRunAntiSoftlock = true;
			}
			else if (ulKey == 0x4C && ms_bToggleModShortcutEnabled) // L
			{
				ms_bDisableMod = !ms_bDisableMod;
			}
			else if (ulKey == 0x4F) // O
			{
				if (c_bIsShiftPressed)
				{
					ms_vSpawnLocation = Vector3();

					if (ComponentExists<SplashTexts>())
					{
						GetComponent<SplashTexts>()->ShowSplash("Spawnpoint disabled", { .86f, .86f }, .8f, { 75, 0, 130 });
					}
				}
				else
				{
					ms_vSpawnLocation = GET_ENTITY_COORDS(PLAYER_PED_ID(), false);

					if (ComponentExists<SplashTexts>())
					{
						GetComponent<SplashTexts>()->ShowSplash("Spawnpoint set", { .86f, .86f }, .8f, { 75, 0, 130 });
					}
				}
			}
		}

		if (ComponentExists<DebugMenu>())
		{
			GetComponent<DebugMenu>()->HandleInput(ulKey, bWasDownBefore);
		}

		if (ComponentExists<Shortcuts>())
		{
			GetComponent<Shortcuts>()->HandleInput(ulKey, bWasDownBefore);
		}
	}
}
