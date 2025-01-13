#include "stdafx.h"
#include "ZChaosManager.h"
#include "EffectDispatcher.h"
#include "Effects/Effect.h"
#include "Effects/EffectData.h"
#include "Effects/EnabledEffectsMap.h"

static void (*ResetFunction)();

static void (*OG_UpdateEffects)(int cycleType);
static void HK_UpdateEffects(int cycleType)
{
	OG_UpdateEffects(cycleType);
	GetComponent<ZChaosManager>()->RunEffectsOnTick(cycleType);
}

static __int64 (*OG_StartEffect)(ZChaosManager::ZChaosEffect* effect);
static __int64 HK_StartEffect(ZChaosManager::ZChaosEffect *effect)
{
	LOG("Trying to trigger effect " << effect->name);
	const auto effectId = GetComponent<ZChaosManager>()->GetIdForEffect(effect);
	if ((!GetRegisteredEffect(effectId) || !g_dictEnabledEffects.contains(effectId)) && !effect->enabledManually)
	{
		return 0;
	}
	if (!GetRegisteredEffect(effectId))
	{
		GetComponent<ZChaosManager>()->RegisterZChaosEffect(effect, effectId);
	}
	if (!g_dictEnabledEffects.contains(effectId))
	{
		GetComponent<ZChaosManager>()->EnableEffect(effect, effectId);
	}
	GetComponent<EffectDispatcher>()->DispatchEffect(effectId);

	return 0;
}

static std::vector<std::pair<const char *, const char *>> labelOverrides = {
	{ "~r~CHAOS WARNING", "~r~There used to be a warning here... right?" },
	{ "~r~AAAAAAAAAAAAA", "~r~AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" },
	{ "~r~CHAOS WARNING", "~r~Something version something something, no one cares" },
	{ "~r~Oh no!",        "~r~The game crashed, you'll have to start over again" },
	{ "~r~warning",       "~r~not so scary now, huh?" },
	{ "~r~BOO!",          "~r~The above text is very spooky, be scared" },
	{ "~b~I'm blue",      "~b~Da ba dee da ba dae" },
	{ "~r~DID YOU KNOW",  "~r~THAT A JIFFY IS AN ACTUAL MEASUREMENT OF TIME???" },
	{ "~r~blalala",       "~r~blelelelelelelele" },
	{ "~r~PLINK",         "~r~OR BUHH????" },
};

void ZChaosManager::RegisterZChaosEffect(ZChaosEffect *effect, std::string effectId)
{
	if (effect->m_OnTick)
	{
		g_RegisteredEffects.emplace_back(
		    effectId,
		    [this, effect, effectId]() -> void
		    {
			    ZChaosEffect e(*effect);
			    m_rgActiveZChaosEffects.emplace(effectId, e);
			    if (e.m_OnStart)
			    {
				    e.m_OnStart(&e);
			    }
			    e.cutoutTime = 0;
		    },
		    [this, effectId]() -> void
		    {
			    auto effect        = &m_rgActiveZChaosEffects.at(effectId);
			    effect->time       = -1000;
			    effect->cutoutTime = 0;
		    },
		    [this, effectId]() -> void
		    {
			    auto effect  = &m_rgActiveZChaosEffects.at(effectId);
			    effect->time = GetComponent<EffectDispatcher>()->GetRemainingTimeForEffect(effectId) * 1000;
		    });
	}
	else
	{
		g_RegisteredEffects.emplace_back(
		    effectId,
		    [this, effect, effectId]() -> void
		    {
			    ZChaosEffect e(*effect);
			    if (e.m_OnStart)
			    {
				    e.m_OnStart(&e);
			    }
			    e.cutoutTime = 0;
		    },
		    nullptr, nullptr);
	}
}

void ZChaosManager::EnableEffect(ZChaosManager::ZChaosEffect* effect, std::string effectId)
{
	EffectData effectData;
	effectData.Name      = effect->name;
	effectData.Id        = effectId;
	effectData.TimedType = effect->m_OnTick ? EEffectTimedType::Custom : EEffectTimedType::NotTimed;
	if (effect->m_OnTick)
	{
		effectData.CustomTime = effect->defaultTime / 1000;
	}

	g_dictEnabledEffects.emplace(effectId, effectData);
}

void ZChaosManager::CheckAndAddEffects()
{
	for (int i = 0; i < m_iEffectCount; ++i)
	{
		ZChaosEffect *effect = m_rgEffectsArray[i];

		if (!effect)
		{
			LOG("No effect at index " << i);
			continue;
		}

		const auto effectId = GetIdForEffect(effect, i);

		if ((!IsEffectEnabled || IsEffectEnabled(effect, false)) && effect->enabledManually)
		{
			if (!g_dictEnabledEffects.contains(effectId))
			{
				EnableEffect(effect, effectId);
				LOG("Effect " << effect->name << " has been enabled");
			}
		}
		else if (g_dictEnabledEffects.contains(effectId))
		{
			g_dictEnabledEffects.erase(effectId);
			LOG("Effect " << effect->name << " has been disabled");
		}

		if (GetRegisteredEffect(effectId))
		{
			continue;
		}

		RegisterZChaosEffect(effect, effectId);

		LOG("Registered ZChaos effect \"" << effect->name << "\" with id " << effectId);
	}
}

std::string ZChaosManager::GetIdForEffect(ZChaosEffect *effect)
{
	for (int i = 0; i < m_iEffectCount; ++i)
	{
		if (effect->name == m_rgEffectsArray[i]->name)
		{
			return GetIdForEffect(effect, i);
		}
	}

	return GetIdForEffect(effect, 100000);
}

std::string ZChaosManager::GetIdForEffect(ZChaosEffect *effect, int index)
{
	std::ostringstream oss;
	oss << "zchaos_" << index << "_";
	char *ptr = effect->name;
	for (char c = *ptr; c; c = *++ptr)
	{
		if ('a' <= c && c <= 'z')
		{
			oss << c;
		}
		if ('A' <= c && c <= 'Z')
		{
			oss << (char)(c - 'A' + 'a');
		}
	}

	return oss.str();
}

static void OverrideStr(char *orig, const char *newValue)
{
	int i = 0;
	for (; newValue[i]; ++i)
	{
		Memory::Write(orig + i, newValue[i]);
	}
	Memory::Write(orig + i, '\0');
}

void ZChaosManager::OverrideWarning()
{
	*m_pWarningTime = 8;
	const auto pair = labelOverrides[g_Random.GetRandomInt(0, labelOverrides.size() - 1)];

	OverrideStr(m_pWarningStr, pair.first);
	OverrideStr(m_pWarningStr2, pair.second);
}

ZChaosManager::ZChaosManager()
    : m_iEffectCount(0),
      m_rgEffectsArray(nullptr),
      IsEffectEnabled(nullptr),
      m_pDisableChaosUI(nullptr),
      m_pNoTimer(nullptr),
      m_pZChaosActive(nullptr),
      m_pWarningTime(nullptr),
      m_pWarningStr(nullptr),
      m_pWarningStr2(nullptr)
{
	auto lib = GetModuleHandle(L"ZChaosV.asi");
	if (!lib)
	{
		LOG("Could not found ZChaos lib! Check that ZChaos is installed");
		return;
	}

	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), lib, &moduleInfo, sizeof(moduleInfo));

	DWORD64 startAddr = reinterpret_cast<DWORD64>(moduleInfo.lpBaseOfDll);

	Memory::PatternScanRange range = { startAddr, startAddr + moduleInfo.SizeOfImage };
	
	auto handle = Memory::FindPattern("48 83 EC 28 0F 57", range);
	if (handle.IsValid())
	{
		ResetFunction = handle.Get<void()>();
	}

	handle = Memory::FindPattern("E8 ?? ?? ?? ?? 40 38 35 ?? ?? ?? ?? 0F 84 95 00 00 00", range);
	if (handle.IsValid())
	{
		Memory::AddHook(handle.Into().Get<void()>(), HK_UpdateEffects, &OG_UpdateEffects);
		LOG("Added update effects hook");
	}

	handle            = Memory::FindPattern("0F B6 3D ?? ?? ?? ?? 8B 05", range);
	m_pZChaosActive   = handle.At(2).Into().Get<char>();

	handle            = Memory::FindPattern("44 38 3D ?? ?? ?? ?? 75 09 41 0F 28 C2", range);
	m_pDisableChaosUI = handle.At(2).Into().Get<char>();

	handle            = Memory::FindPattern("44 38 3D ?? ?? ?? ?? 0F 85 48 03 00 00", range);
	m_pNoTimer        = handle.At(2).Into().Get<char>();

	handle            = Memory::FindPattern("F2 0F 10 0D ?? ?? ?? ?? 66 41 0F 2F C8 0F 86 00 01 00 00", range);
	m_pWarningTime    = handle.At(3).Into().Get<double>();

	handle            = Memory::FindPattern("48 8D 05 ?? ?? ?? ?? 48 89 44 24 60 BF FF 00 00 00", range);
	m_pWarningStr     = handle.At(2).Into().Get<char>();

	handle            = Memory::FindPattern("48 8D 05 ?? ?? ?? ?? 48 89 44 24 60 89 7C 24 58", range);
	m_pWarningStr2    = handle.At(2).Into().Get<char>();

	handle            = Memory::FindPattern("40 53 48 83 EC 20 33 C0 48", range);
	IsEffectEnabled   = handle.Get<bool(ZChaosEffect *, bool)>();

	auto GetZChaosVersion = (void (*)(int *version, int *success))GetProcAddress(lib, "GetZChaosVersion");
	auto GetZChaosEffectCount = (int (*)())GetProcAddress(lib, "GetZChaosEffectCount");
	auto TriggerZChaosEffect  = (int (*)(int index))GetProcAddress(lib, "TriggerZChaosEffect");
	auto TriggerZChaosEffectByPointer = (int (*)(int index))GetProcAddress(lib, "TriggerZChaosEffectByPointer");
	auto DoesZChaosEffectHaveTimer = (bool (*)(int index))GetProcAddress(lib, "DoesZChaosEffectHaveTimer");
	
	int version;
	int success;
	GetZChaosVersion(&version, &success);
	m_iEffectCount = GetZChaosEffectCount();
	LOG("ZChaos Version: " << version << " Effect Count: " << m_iEffectCount);

	if (version != 240103)
	{
		LOG("Warning: Potentially unsupported chaos version!");
	}

	handle = Handle(reinterpret_cast<uintptr_t>(TriggerZChaosEffect));
	m_rgEffectsArray = handle.At(13).Into().Get<ZChaosEffectsList>()->first;

	handle           = Memory::FindPattern("E8 ?? ?? ?? ?? B1 01 E8 ?? ?? ?? ?? 48 8B C8 E8", range);
	if (handle.IsValid())
	{
		Memory::AddHook(handle.Into().Get<void()>(), HK_StartEffect, &OG_StartEffect);
		LOG("Added start effect hook");
	}

	if (m_pWarningTime && *m_pWarningTime > 0)
	{
		OverrideWarning();
	}
	CheckAndAddEffects();
}

ZChaosManager::~ZChaosManager()
{
	OnModPauseCleanup();
}

void ZChaosManager::OnRun()
{
	if (m_pZChaosActive)
	{
		*m_pZChaosActive = true;
	}
	if (m_pNoTimer)
	{
		*m_pNoTimer      = true;
	}
	if (m_pDisableChaosUI)
	{
		//*m_pDisableChaosUI = true;
	}
}

void ZChaosManager::OnModPauseCleanup()
{
	for (int i = 0; i < m_iEffectCount; ++i)
	{
		ZChaosEffect *effect = m_rgEffectsArray[i];

		if (!effect)
		{
			LOG("No effect at index " << i);
			continue;
		}

		std::string effectId = GetIdForEffect(effect);
				
		if (g_dictEnabledEffects.contains(effectId))
		{
			g_dictEnabledEffects.erase(effectId);
		}

		if (auto reg = GetRegisteredEffect(effectId))
		{
			g_RegisteredEffects.erase(std::remove_if(g_RegisteredEffects.begin(), g_RegisteredEffects.end(),
			                                         [effectId](RegisteredEffect e)
			                                         { return e.GetIndentifier().GetEffectId() == effectId; }),
			                          g_RegisteredEffects.end());
		}
	}

	
	m_rgEffectsArray = 0;
	m_iEffectCount   = 0;
}

void ZChaosManager::RunEffectsOnTick(int cycleType)
{
	for (auto it = m_rgActiveZChaosEffects.begin(); it != m_rgActiveZChaosEffects.end();)
	{
		auto &[effectId, effect] = *it;
		bool flag = false;

		if (effect.m_OnTick && effect.cycleType == cycleType)
		{
			effect.m_OnTick(&effect);
			if ((double)effect.time * 0.001 <= effect.cutoutTime)
			{
				flag = true;
			}
		}

		if (flag)
		{
			it = m_rgActiveZChaosEffects.erase(it);
			CheckAndAddEffects();
		}
		else
		{
			++it;
		}
	}
}
