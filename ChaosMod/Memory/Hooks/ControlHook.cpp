#include <stdafx.h>

#include "ControlHook.h"

#include "Memory/Shader.h"

#include "Memory/Hooks/GameSpeedHook.h"
#include "Memory/Hooks/Hook.h"

const int MAX_CONTROLS = 360;

struct ioValue
{
	float m_InvertValue;
	float m_Value;
	float m_LastValue;
	float m_NextFrameValue;

	char m_Source[12];
	char m_LastSource[12];
	char m_NextFrameSource[12];

	char pad[12];

	enum
	{
		MaxHistoryValues = 64
	};
	struct HistoryValue
	{
		float m_value;
		int m_timeMS;
		char m_source[12];
	};

	HistoryValue *m_HistoryValues;
};

static_assert(sizeof(ioValue) == 0x48);

struct Update
{
	enum UpdateType
	{
		GAME,
		ROUTE
	} m_UpdateType;

	float m_LastGameValue;
};

static ioValue *ms_Controls;
static ioValue ms_ControlsCopy[MAX_CONTROLS];
static Update ms_ControlsState[MAX_CONTROLS];

struct RouteRule
{
	int destination;
	std::function<float(float)> transform;

	RouteRule(int dest)
	{
		destination = dest;
		transform   = [](float x)
		{
			return x;
		};
	}

	RouteRule(int dest, std::function<float(float)> trans)
	{
		destination = dest;
		transform   = trans;
	}
};

static std::mutex ms_UpdateMutex;

static std::map<int, std::vector<RouteRule>> ms_Routes;

void Hooks::RouteKey(int ogAction, int routeAction)
{
	std::lock_guard lock(ms_UpdateMutex);

	if (!ms_Routes.contains(ogAction))
		ms_Routes.emplace(ogAction, std::vector<RouteRule>());
	ms_Routes[ogAction].push_back(RouteRule(routeAction));
}

void Hooks::RouteKeyWithTransform(int ogAction, int routeAction, std::function<float(float)> transform)
{
	std::lock_guard lock(ms_UpdateMutex);

	if (!ms_Routes.contains(ogAction))
		ms_Routes.emplace(ogAction, std::vector<RouteRule>());
	ms_Routes[ogAction].push_back(RouteRule(routeAction, transform));
}

void Hooks::ResetKeyRoute(int ogAction)
{
	std::lock_guard lock(ms_UpdateMutex);

	ms_Routes.erase(ogAction);
}

void Hooks::ResetKeyRoutes()
{
	std::lock_guard lock(ms_UpdateMutex);

	ms_Routes.clear();
}

static void (*OG_rage__ioValue__SetCurrentValue)(ioValue *_this, float value, const void *source);
static void HK_rage__ioValue__SetCurrentValue(ioValue *_this, float value, const void *source)
{
	// unused

	OG_rage__ioValue__SetCurrentValue(_this, value, source);
}

static void (*OG_rage__ioValue__SetNextValue)(ioValue *_this, float value, const void *source);
static void HK_rage__ioValue__SetNextValue(ioValue *_this, float value, const void *source)
{
	// unused

	OG_rage__ioValue__SetNextValue(_this, value, source);
}

static void OnUpdate()
{
	if (!ms_Controls)
		return;

	std::lock_guard lock(ms_UpdateMutex);

	std::set<int> destinations;
	for (const auto &[src, route] : ms_Routes)
		for (const auto &[destination, _] : route)
			destinations.emplace(destination);

	for (int i = 0; i < MAX_CONTROLS; ++i)
	{
		bool updated = false;
		for (int j = 0; j < sizeof(ioValue); ++j)
		{
			bool eq = reinterpret_cast<byte *>(&ms_Controls[i])[j] == reinterpret_cast<byte *>(&ms_ControlsCopy[i])[j];

			// do not include the m_LastUpdateID field
			if ((j < 56 || j > 58) ? !eq : !eq && ms_ControlsState[i].m_LastGameValue != ms_Controls[i].m_Value)
			{
				updated                             = true;
				ms_ControlsState[i].m_UpdateType    = Update::GAME;
				ms_ControlsState[i].m_LastGameValue = ms_Controls[i].m_Value;
				break;
			}
		}

		if (updated && ms_Routes.contains(i))
		{
			for (const auto &[destination, transform] : ms_Routes[i])
			{
				if (ms_ControlsState[destination].m_UpdateType == Update::GAME
				    || std::abs(ms_Controls[i].m_Value) > std::abs(ms_ControlsCopy[destination].m_Value))
				{
					OG_rage__ioValue__SetCurrentValue(&ms_ControlsCopy[destination], transform(ms_Controls[i].m_Value),
					                                  &ms_Controls[i].m_Source);
					OG_rage__ioValue__SetNextValue(&ms_ControlsCopy[destination],
					                               transform(ms_Controls[i].m_NextFrameValue),
					                               &ms_Controls[i].m_NextFrameSource);
				}

				ms_ControlsState[destination].m_UpdateType = Update::ROUTE;
			}
		}

		if (!destinations.contains(i))
			memcpy_s(&ms_ControlsCopy[i], sizeof(ioValue), &ms_Controls[i], sizeof(ioValue));
	}

	memcpy_s(ms_Controls, sizeof(ioValue) * MAX_CONTROLS, ms_ControlsCopy, sizeof(ioValue) * MAX_CONTROLS);
}

static void (*OG_rage__ioMapper__Update)(void *_this, unsigned int timeMS, bool forceKeyboardMouse, float formal);
static void HK_rage__ioMapper__Update(void *_this, unsigned int timeMS, bool forceKeyboardMouse, float formal)
{
	OG_rage__ioMapper__Update(_this, timeMS, forceKeyboardMouse, formal);

	OnUpdate();
}

static bool OnHook()
{
	Handle handle    = Memory::FindPattern("48 8D 88 ?? ?? ?? ?? F3 44 0F 10 05");
	const int offset = handle.At(3).Value<int>() - 83 * sizeof(ioValue);

	handle           = Memory::FindPattern("48 8D 35 ?? ?? ?? ?? 48 89 F0 48 83 C4 28 5B 5E");
	ms_Controls      = handle.At(2).Into().At(offset).Get<ioValue>();

	handle           = Memory::FindPattern("E8 ?? ?? ?? ?? 8B 1D ?? ?? ?? ?? 48 89 F1");
	Memory::AddHook(handle.Into().Get<void>(), HK_rage__ioValue__SetCurrentValue, &OG_rage__ioValue__SetCurrentValue);

	handle = Memory::FindPattern("F3 0F 11 49 0C 49");
	Memory::AddHook(handle.Get<void>(), HK_rage__ioValue__SetNextValue, &OG_rage__ioValue__SetNextValue);

	handle = Memory::FindPattern("E8 ?? ?? ?? ?? 66 0F 6F 05 ?? ?? ?? ?? 66 0F EF 05");
	Memory::AddHook(handle.Into().Get<void>(), HK_rage__ioMapper__Update, &OG_rage__ioMapper__Update);

	return true;
}

static void OnCleanup()
{
	ms_Controls = 0;
	ms_Routes.clear();
}

static RegisterHook registerHook(OnHook, OnCleanup, "ControlHook", true);