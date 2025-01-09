#include "stdafx.h"
#include "ZChaosManager.h"
#include "EffectDispatcher.h"
#include "Effects/Effect.h"
#include "Effects/EffectData.h"
#include "Effects/EnabledEffectsMap.h"

static void (*ResetFunction)();

static void (*OG_UpdateEffects)(int a);
static void HK_UpdateEffects(int a)
{
	OG_UpdateEffects(a);
	GetComponent<ZChaosManager>()->RunEffectsOnTick(a);
}

static __int64 (*OG_StartEffect)(__int64 a);
static __int64 HK_StartEffect(__int64 a)
{
	return 0;
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

		std::ostringstream oss;
		oss << "zchaos_" << i << "_";
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

		std::string effectId = oss.str();


		EffectData effectData;
		effectData.Name      = effect->name;
		effectData.Id        = effectId;
		effectData.TimedType = effect->m_OnTick ? EEffectTimedType::Custom : EEffectTimedType::NotTimed;
		if (effect->m_OnTick)
		{
			effectData.CustomTime = effect->defaultTime / 1000;
		}

		if ((!IsEffectEnabled || IsEffectEnabled(effect, false)))
		{
			g_dictEnabledEffects.emplace(effectId, effectData);
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

		LOG("Registered ZChaos effect \"" << effect->name << "\" with id " << effectId);
	}
}

ZChaosManager::ZChaosManager()
    : m_iEffectCount(0),
      m_rgEffectsArray(nullptr),
      IsEffectEnabled(nullptr),
      m_pDisableChaosUI(nullptr),
      m_pNoTimer(nullptr),
      m_pZChaosActive(nullptr)
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

	handle                 = Memory::FindPattern("0F B6 3D ?? ?? ?? ?? 8B 05", range);
	m_pZChaosActive   = handle.At(2).Into().Get<char>();

	handle                 = Memory::FindPattern("44 38 3D ?? ?? ?? ?? 75 09 41 0F 28 C2", range);
	m_pDisableChaosUI = handle.At(2).Into().Get<char>();

	handle                 = Memory::FindPattern("44 38 3D ?? ?? ?? ?? 0F 85 48 03 00 00", range);
	m_pNoTimer        = handle.At(2).Into().Get<char>();

	handle                 = Memory::FindPattern("40 53 48 83 EC 20 33 C0 48", range);
	IsEffectEnabled   = handle.Get<bool(ZChaosEffect *, bool)>();

	LOG(IsEffectEnabled);

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
	m_rgEffectsArray = 0;
	m_iEffectCount   = 0;
}

void ZChaosManager::RunEffectsOnTick(int a)
{
	for (auto it = m_rgActiveZChaosEffects.begin(); it != m_rgActiveZChaosEffects.end();)
	{
		auto &[effectId, effect] = *it;
		bool flag = false;

		if (effect.m_OnTick && effect.field_28 == a)
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
