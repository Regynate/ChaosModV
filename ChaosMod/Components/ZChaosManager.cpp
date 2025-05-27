#include "EffectDispatcher.h"
#include "Effects/Register/RegisteredEffectsMetadata.h"
#include "Util/OptionsFile.h"

#include "ZChaosManager.h"

#include "stdafx.h"

static void (*ResetFunction)();

static void (*OG_UpdateEffects)(int cycleType);
static void HK_UpdateEffects(int cycleType)
{
	OG_UpdateEffects(cycleType);
	GetComponent<ZChaosManager>()->RunEffectsOnTick(cycleType);
}

static __int64 (*OG_StartEffect)(ZChaosManager::ZChaosEffect *effect);
static __int64 HK_StartEffect(ZChaosManager::ZChaosEffect *effect)
{
	LOG("Trying to trigger effect " << effect->name);
	const auto effectId = GetComponent<ZChaosManager>()->GetIdForEffect(effect);
	if (!GetRegisteredEffect(effectId) || !g_EnabledEffects.contains(effectId) || !effect->enabledManually)
		return 0;
	GetComponent<EffectDispatcher>()->DispatchEffect(effectId);

	return 0;
}

static std::vector<std::pair<const char *, const char *>> labelOverrides = {
	{ "~r~AAAAAAAAAAAAA", "~r~AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" },
	{ "~r~CHAOS WARNING", "~r~There used to be a warning here... right?" },
	{ "~r~OH NO", "~r~You are doing bad thing, please only do good thing" },
	{ "~r~warning", "~r~not so scary now, huh?" },
	{ "~r~BOO!", "~r~The above text is very spooky, be scared" },
	{ "~b~I'm blue", "~b~Da ba dee da ba dae" },
	{ "~r~blalala", "~r~blelelelelelelele" },
	{ "~r~hello", "~r~your computer has virus" },
};

static void RemoveEffect(std::string effectId)
{
	if (g_EnabledEffects.contains(effectId))
		g_EnabledEffects.erase(effectId);

	if (auto reg = GetRegisteredEffect(effectId))
	{
		g_RegisteredEffects.erase(std::remove_if(g_RegisteredEffects.begin(), g_RegisteredEffects.end(),
		                                         [effectId](RegisteredEffect e) { return e.GetId().Id() == effectId; }),
		                          g_RegisteredEffects.end());
	}
}

void ZChaosManager::RegisterZChaosEffect(ZChaosEffect *effect, std::string effectId)
{
	if (effect->m_OnTick)
	{
		g_RegisteredEffects.emplace_back(
		    effectId,
		    [this, effect, effectId]() -> void
		    {
			    ZChaosEffect e(*effect);
			    m_ActiveZChaosEffects.emplace(effectId, e);
			    if (e.m_OnStart)
				    e.m_OnStart(&e);
			    e.cutoutTime = 0;
		    },
		    [this, effectId]() -> void
		    {
			    auto effect        = &m_ActiveZChaosEffects.at(effectId);
			    effect->time       = -1000;
			    effect->cutoutTime = 0;
		    },
		    [this, effectId]() -> void
		    {
			    if (m_ActiveZChaosEffects.contains(effectId))
			    {
				    auto effect  = &m_ActiveZChaosEffects.at(effectId);
				    effect->time = (int)(GetComponent<EffectDispatcher>()->GetRemainingTimeForEffect(effectId) * 1000);
			    }
		    },
		    nullptr);
	}
	else
	{
		g_RegisteredEffects.emplace_back(
		    effectId,
		    [this, effect, effectId]() -> void
		    {
			    ZChaosEffect e(*effect);
			    if (e.m_OnStart)
				    e.m_OnStart(&e);
			    e.cutoutTime = 0;
		    },
		    nullptr, nullptr, nullptr);
	}
}

EffectData ZChaosManager::GetDefaultEffectData(ZChaosManager::ZChaosEffect *effect, std::string effectId)
{
	EffectData effectData;
	effectData.Name      = effect->name;
	effectData.Id        = effectId;
	effectData.TimedType = effect->m_OnTick ? EffectTimedType::Custom : EffectTimedType::NotTimed;
	if (effect->m_OnTick)
		effectData.CustomTime = (float)effect->defaultTime / 1000;

	return effectData;
}

void ZChaosManager::EnableEffect(std::string effectId, EffectData effectData)
{
	g_EnabledEffects.emplace(effectId, effectData);
}

static std::string ReplaceAll(std::string str, const std::string &from, const std::string &to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

void ZChaosManager::AddEffects()
{
	m_EnabledEffects.clear();

	OptionsFile effectsFile("chaosmod/configs/zchaos.ini");

	for (int i = 0; i < m_EffectCount; ++i)
	{
		ZChaosEffect *effect = m_EffectsArray[i];

		if (!effect)
		{
			LOG("No effect at index " << i);
			continue;
		}

		const auto effectId    = GetIdForEffect(effect);

		const auto defaultData = GetDefaultEffectData(effect, effectId);
		RegisteredEffectMetadata effectMetadata { .Name    = defaultData.Name,
			                                      .Id      = effectId,
			                                      .IsTimed = defaultData.TimedType != EffectTimedType::NotTimed };

		EffectData effectData;

		if (EffectConfig::GetConfigFromMetadata(effectsFile, effectId, effectMetadata, effectData))
		{
			if (!GetRegisteredEffect(effectId))
			{
				RegisterZChaosEffect(effect, effectId);
				LOG("Registered ZChaos effect \"" << effect->name << "\" with id " << effectId);
			}

			// zchaos effects don't have timed types; hack
			if (effectData.TimedType == EffectTimedType::Normal)
			{
				effectData.TimedType  = EffectTimedType::Custom;
				effectData.CustomTime = defaultData.CustomTime;
			}

			EnableEffect(effectId, effectData);
		}
	}
}

std::string ZChaosManager::GetIdForEffect(ZChaosEffect *effect)
{
	std::ostringstream oss;
	oss << "zchaos_";
	char *ptr = effect->name;
	for (char c = *ptr; c; c = *++ptr)
	{
		if (('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || c == '-')
			oss << c;
		if ('A' <= c && c <= 'Z')
			oss << (char)(c - 'A' + 'a');
		if (c == ' ')
			oss << '_';
	}

	return oss.str();
}

static void OverrideStr(char *orig, const char *newValue)
{
	int i = 0;
	for (; newValue[i]; ++i)
		Memory::Write(orig + i, newValue[i]);
	Memory::Write(orig + i, '\0');
}

void ZChaosManager::OverrideWarning()
{
	*m_WarningTime  = 0;
	const auto pair = labelOverrides[g_Random.GetRandomInt(0, labelOverrides.size() - 1)];

	OverrideStr(m_WarningStr, pair.first);
	OverrideStr(m_WarningStr2, pair.second);
}

ZChaosManager::ZChaosManager()
    : m_EffectCount(0),
      m_EffectsArray(nullptr),
      IsEffectEnabled(nullptr),
      m_DisableChaosUI(nullptr),
      m_NoTimer(nullptr),
      m_ZChaosActive(nullptr),
      m_WarningTime(nullptr),
      m_WarningStr(nullptr),
      m_WarningStr2(nullptr)
{
	auto lib = GetModuleHandle(L"ZChaosV.asi");
	if (!lib)
	{
		LOG("Could not find ZChaos lib! Check that ZChaos is installed");
		return;
	}

	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), lib, &moduleInfo, sizeof(moduleInfo));

	DWORD64 startAddr              = reinterpret_cast<DWORD64>(moduleInfo.lpBaseOfDll);

	Memory::PatternScanRange range = { startAddr, startAddr + moduleInfo.SizeOfImage };

	auto handle                    = Memory::FindPattern("48 83 EC 28 0F 57", range);
	if (handle.IsValid())
		ResetFunction = handle.Get<void()>();

	handle = Memory::FindPattern("E8 ?? ?? ?? ?? 40 38 35 ?? ?? ?? ?? 0F 84 95 00 00 00", range);
	if (handle.IsValid())
	{
		Memory::AddHook(handle.Into().Get<void()>(), HK_UpdateEffects, &OG_UpdateEffects);
		LOG("Added update effects hook");
	}

	handle                    = Memory::FindPattern("0F B6 3D ?? ?? ?? ?? 8B 05", range);
	m_ZChaosActive            = handle.At(2).Into().Get<char>();

	handle                    = Memory::FindPattern("44 38 3D ?? ?? ?? ?? 75 09 41 0F 28 C2", range);
	m_DisableChaosUI          = handle.At(2).Into().Get<char>();

	handle                    = Memory::FindPattern("44 38 3D ?? ?? ?? ?? 0F 85 48 03 00 00", range);
	m_NoTimer                 = handle.At(2).Into().Get<char>();

	handle                    = Memory::FindPattern("F2 0F 10 0D ?? ?? ?? ?? 66 41 0F 2F C8 0F 86 00 01 00 00", range);
	m_WarningTime             = handle.At(3).Into().Get<double>();

	handle                    = Memory::FindPattern("48 8D 05 ?? ?? ?? ?? 48 89 44 24 60 BF FF 00 00 00", range);
	m_WarningStr              = handle.At(2).Into().Get<char>();

	handle                    = Memory::FindPattern("48 8D 05 ?? ?? ?? ?? 48 89 44 24 60 89 7C 24 58", range);
	m_WarningStr2             = handle.At(2).Into().Get<char>();

	handle                    = Memory::FindPattern("40 53 48 83 EC 20 33 C0 48", range);
	IsEffectEnabled           = handle.Get<bool(ZChaosEffect *, bool)>();

	auto GetZChaosVersion     = (void (*)(int *version, int *success))GetProcAddress(lib, "GetZChaosVersion");
	auto GetZChaosEffectCount = (int (*)())GetProcAddress(lib, "GetZChaosEffectCount");
	auto TriggerZChaosEffect  = (int (*)(int index))GetProcAddress(lib, "TriggerZChaosEffect");
	auto TriggerZChaosEffectByPointer = (int (*)(int index))GetProcAddress(lib, "TriggerZChaosEffectByPointer");
	auto DoesZChaosEffectHaveTimer    = (bool (*)(int index))GetProcAddress(lib, "DoesZChaosEffectHaveTimer");

	int version;
	int success;
	GetZChaosVersion(&version, &success);
	m_EffectCount = GetZChaosEffectCount();
	LOG(std::dec << "ZChaos Version: " << version << " Effect Count: " << m_EffectCount);

	if (version != 240103)
		LOG("Warning: Potentially unsupported zchaos version!");

	handle         = Handle(reinterpret_cast<uintptr_t>(TriggerZChaosEffect));
	m_EffectsArray = handle.At(13).Into().Get<ZChaosEffectsList>()->first;

	handle         = Memory::FindPattern("E8 ?? ?? ?? ?? B1 01 E8 ?? ?? ?? ?? 48 8B C8 E8", range);
	if (handle.IsValid())
	{
		Memory::AddHook(handle.Into().Get<void()>(), HK_StartEffect, &OG_StartEffect);
		LOG("Added start effect hook");
	}

	if (m_WarningTime && *m_WarningTime > 0)
		OverrideWarning();
	AddEffects();
}

void ZChaosManager::OnRun()
{
	if (m_ZChaosActive)
		*m_ZChaosActive = true;
	if (m_NoTimer)
		*m_NoTimer = true;
	if (m_DisableChaosUI)
	{
		//*m_DisableChaosUI = true;
	}
}

void ZChaosManager::OnModPauseCleanup()
{
	for (int i = 0; i < m_EffectCount; ++i)
	{
		ZChaosEffect *effect = m_EffectsArray[i];

		if (!effect)
		{
			LOG("No effect at index " << i);
			continue;
		}

		std::string effectId = GetIdForEffect(effect);
		RemoveEffect(effectId);
	}

	m_EffectsArray = 0;
	m_EffectCount  = 0;
}

void ZChaosManager::RunEffectsOnTick(int cycleType)
{
	for (auto it = m_ActiveZChaosEffects.begin(); it != m_ActiveZChaosEffects.end();)
	{
		auto &[effectId, effect] = *it;
		bool flag                = false;

		if (effect.m_OnTick && effect.cycleType == cycleType)
		{
			effect.m_OnTick(&effect);
			if ((double)effect.time * 0.001 <= effect.cutoutTime)
				flag = true;
		}

		if (flag)
		{
			GetComponent<EffectDispatcher>()->ClearEffect(effectId);
			it = m_ActiveZChaosEffects.erase(it);
		}
		else
			++it;
	}
}
