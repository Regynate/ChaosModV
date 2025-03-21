#include <stdafx.h>

#include "Components/CrossingChallenge.h"
#include "Components/EffectDispatchTimer.h"
#include "Components/EffectDispatcher.h"
#include "Components/EffectSound/EffectSoundManager.h"
#include "Components/MetaModifiers.h"
#include "EffectDispatcher.h"
#include "Effects/EffectCategory.h"
#include "Effects/EffectTimedType.h"
#include "Effects/EnabledEffects.h"
#include "Effects/Register/RegisteredEffectsMetadata.h"
#include "Util/OptionsManager.h"
#include "Util/Random.h"
#include "Util/ScriptText.h"

#define EFFECT_TEXT_INNER_SPACING_MIN .030f
#define EFFECT_TEXT_INNER_SPACING_MAX .075f
#define EFFECT_TEXT_TOP_SPACING .2f
#define EFFECT_TEXT_TOP_SPACING_EXTRA .35f
#define EFFECT_NONTIMED_TIMER_SPEEDUP_MIN_EFFECTS 3

static void _DispatchEffect(EffectDispatcher *effectDispatcher, const EffectDispatcher::EffectDispatchEntry &entry)
{
	auto &effectData = g_EnabledEffects.at(entry.Id);

	if (effectData.TimedType == EffectTimedType::Permanent)
		return;

	if (!effectDispatcher->OnPreDispatchEffect.Fire(entry.Id))
		return;

#ifdef CHAOSDEBUG
	DEBUG_LOG("Dispatching effect \"" << effectData.Name << "\"" << " (" << effectData.Id.Id() << ")");
#else
	LOG("Dispatching effect \"" << effectData.Name << "\"");
#endif

	// Increase weight for all effects first
	for (auto &[effectId, enabledEffectData] : g_EnabledEffects)
		if (!enabledEffectData.IsMeta())
			enabledEffectData.Weight += enabledEffectData.WeightMult;

	// Reset weight of this effect (or every effect in group) to reduce chance of same effect (group) happening multiple
	// times in a row
	if (effectData.GroupType.empty())
		effectData.Weight = effectData.WeightMult;
	else
	{
		for (auto &[effectId, enabledEffectData] : g_EnabledEffects)
			if (enabledEffectData.GroupType == effectData.GroupType)
				effectData.Weight = effectData.WeightMult;
	}

	auto playEffectDispatchSound = [&](EffectDispatcher::ActiveEffect &activeEffect)
	{
		if ((!ComponentExists<MetaModifiers>() || !GetComponent<MetaModifiers>()->HideChaosUI)
		    && ComponentExists<EffectSoundManager>())
		{
			// Play global sound (if one exists)
			// HACK: Force no global sound for "Fake Crash"
			if (entry.Id != "misc_fakecrash")
				GetComponent<EffectSoundManager>()->PlaySoundFile("global_effectdispatch");

			// Play a sound if corresponding .mp3 file exists
			activeEffect.SoundId = GetComponent<EffectSoundManager>()->PlaySoundFile(effectData.Id);
		}
	};

	// Check if timed effect already is active, reset timer if so
	// Also check for incompatible effects
	bool alreadyExists = false;

	for (auto &activeEffect : effectDispatcher->SharedState.ActiveEffects)
	{
		if (activeEffect.Id == entry.Id)
		{
			if (effectData.TimedType != EffectTimedType::NotTimed)
			{
				// Just extend timer of existing instance of timed effect
				alreadyExists      = true;
				activeEffect.Timer = activeEffect.MaxTime;

				playEffectDispatchSound(activeEffect);

				break;
			}
			else
			{
				// Replace previous instance of non-timed effect with this new one
				EffectThreads::StopThreadImmediately(activeEffect.ThreadId);
				activeEffect.IsZombie = true;
			}
		}

		const auto &activeEffectData = g_EnabledEffects.at(activeEffect.Id);

		bool isIncompatible          = false;
		if (effectData.IncompatibleIds.contains(activeEffectData.Id))
			isIncompatible = true;

		// Check if current effect is either the same effect category or marked as incompatible in active effect
		if (!isIncompatible)
		{
			const auto &activeIncompatibleIds = activeEffectData.IncompatibleIds;
			if ((effectData.Category != EffectCategory::None && effectData.Category == activeEffectData.Category)
			    || activeIncompatibleIds.contains(effectData.Id))
			{
				isIncompatible = true;
			}
		}

		if (isIncompatible)
		{
			// No immediate effect thread stopping required here, do it gracefully
			EffectThreads::StopThread(activeEffect.ThreadId);
			activeEffect.IsStopping = true;
		}
	}

	if (!alreadyExists)
	{
		auto *registeredEffect = GetRegisteredEffect(entry.Id);

		if (registeredEffect)
		{
			std::ostringstream effectName;
			effectName << (effectData.HasCustomName() ? effectData.CustomName : effectData.Name);

			if (!entry.Suffix.empty())
				effectName << " " << entry.Suffix;

			float effectDuration = 0;
			switch (effectData.TimedType)
			{
			case EffectTimedType::NotTimed:
				effectDuration = (effectData.IsMeta() ? effectDispatcher->SharedState.MetaEffectTimedDur
				                                      : effectDispatcher->SharedState.EffectTimedDur)
				               * 3.f;
				break;
			case EffectTimedType::Normal:
				effectDuration = effectData.IsMeta() ? effectDispatcher->SharedState.MetaEffectTimedDur
				                                     : effectDispatcher->SharedState.EffectTimedDur;
				break;
			case EffectTimedType::Short:
				effectDuration = effectData.IsMeta() ? effectDispatcher->SharedState.MetaEffectShortDur
				                                     : effectDispatcher->SharedState.EffectTimedShortDur;
				break;
			case EffectTimedType::Custom:
				effectDuration = effectData.CustomTime;
				break;
			default:
				LOG("WARNING: No effectDuration set for effect " << entry.Id.Id() << " with EffectTimedType "
				                                                 << static_cast<int>(effectData.TimedType)
				                                                 << ", reverting to default!");
				effectDuration = effectData.IsMeta() ? effectDispatcher->SharedState.MetaEffectTimedDur
				                                     : effectDispatcher->SharedState.EffectTimedDur;
				break;
			}

			effectDispatcher->SharedState.ActiveEffects.push_back({
			    .Id             = entry.Id,
			    .Name           = effectName.str(),
			    .ThreadId       = EffectThreads::CreateThread(registeredEffect),
			    .Timer          = static_cast<float>(effectDuration),
			    .MaxTime        = static_cast<float>(effectDuration),
			    .IsTimed        = effectData.TimedType != EffectTimedType::NotTimed,
			    .IsMeta         = effectData.IsMeta(),
			    .HideEffectName = effectData.ShouldHideRealNameOnStart(),
			});
			auto &activeEffect = effectDispatcher->SharedState.ActiveEffects.back();

			playEffectDispatchSound(activeEffect);

			// There might be a reason to include meta effects in the future, for now we will just exclude them
			if (!(entry.Flags & EffectDispatcher::DispatchEffectFlag_NoAddToLog) && !effectData.IsMeta()
			    && !effectData.IsTemporary() && !effectData.IsUtility())
			{
				if (effectDispatcher->SharedState.DispatchedEffectsLog.size() >= 100)
				{
					effectDispatcher->SharedState.DispatchedEffectsLog.erase(
					    effectDispatcher->SharedState.DispatchedEffectsLog.begin());
				}

				effectDispatcher->SharedState.DispatchedEffectsLog.emplace_back(registeredEffect);
			}
		}
	}

	effectDispatcher->OnPostDispatchEffect.Fire(entry.Id);
}

static void _OnRunEffects(LPVOID data)
{
	auto effectDispatcher = reinterpret_cast<EffectDispatcher *>(data);
	auto lastTime         = GetTickCount64();
	while (true)
	{
		auto curTime = GetTickCount64();
		float deltaTime =
		    !ComponentExists<EffectDispatchTimer>()
		        ? 0
		        : (curTime - lastTime)
		              * (ComponentExists<MetaModifiers>() ? GetComponent<MetaModifiers>()->EffectDurationModifier
		                                                  : 1.f);
		// The game was paused
		if (deltaTime > 1000.f)
			deltaTime = 0.f;

		lastTime = curTime;

		while (!effectDispatcher->EffectDispatchQueue.empty())
		{
			_DispatchEffect(effectDispatcher, effectDispatcher->EffectDispatchQueue.front());
			effectDispatcher->EffectDispatchQueue.pop();
		}

		effectDispatcher->UpdateEffects(deltaTime);

		if (!ComponentExists<EffectDispatchTimer>() || GetComponent<EffectDispatchTimer>()->IsTimerEnabled())
			effectDispatcher->UpdateMetaEffects(deltaTime);

		SwitchToFiber(g_MainThread);
	}
}

EffectDispatcher::EffectDispatcher(const std::array<BYTE, 3> &textColor, const std::array<BYTE, 3> &effectTimerColor)
    : Component()
{
	m_TextColor        = textColor;
	m_EffectTimerColor = effectTimerColor;

	m_DisableDrawEffectTexts =
	    g_OptionsManager.GetConfigValue({ "DisableEffectTextDraw" }, OPTION_DEFAULT_NO_TEXT_DRAW);

	SharedState.EffectTimedDur = g_OptionsManager.GetConfigValue({ "EffectTimedDur" }, OPTION_DEFAULT_EFFECT_TIMED_DUR);
	SharedState.EffectTimedShortDur =
	    g_OptionsManager.GetConfigValue({ "EffectTimedShortDur" }, OPTION_DEFAULT_EFFECT_SHORT_TIMED_DUR);

	SharedState.MetaEffectSpawnTime =
	    g_OptionsManager.GetConfigValue({ "NewMetaEffectSpawnTime" }, OPTION_DEFAULT_EFFECT_META_SPAWN_TIME);
	SharedState.MetaEffectTimedDur =
	    g_OptionsManager.GetConfigValue({ "MetaEffectDur" }, OPTION_DEFAULT_EFFECT_META_TIMED_DUR);
	SharedState.MetaEffectShortDur =
	    g_OptionsManager.GetConfigValue({ "MetaShortEffectDur" }, OPTION_DEFAULT_EFFECT_META_SHORT_TIMED_DUR);

	m_MaxRunningEffects =
	    g_OptionsManager.GetConfigValue({ "MaxParallelRunningEffects" }, OPTION_DEFAULT_MAX_RUNNING_PARALLEL_EFFECTS);

	Reset();

	for (const auto &[effectId, effectData] : g_EnabledEffects)
	{
		if (!effectData.IsMeta() && !effectData.IsUtility())
		{
			// There's at least 1 enabled non-permanent effect, enable timer
			m_EnableNormalEffectDispatch = true;
			break;
		}
	}

	if (g_EffectDispatcherThread)
		DeleteFiber(g_EffectDispatcherThread);
	g_EffectDispatcherThread = CreateFiber(0, _OnRunEffects, this);
}

void EffectDispatcher::OnModPauseCleanup()
{
	ClearEffects();
}

void EffectDispatcher::OnRun()
{
	if (g_EffectDispatcherThread)
		SwitchToFiber(g_EffectDispatcherThread);

	DrawEffectTexts();
}

void EffectDispatcher::UpdateEffects(float deltaTime)
{
	if (m_ClearEffectsState != ClearEffectsState::None)
	{
		SharedState.ActiveEffects.clear();
		m_PermanentEffects.clear();
		SharedState.DispatchedEffectsLog.clear();

		EffectThreads::StopThreadsImmediately();

		// Ensure player control isn't stuck in disabled state
		SET_PLAYER_CONTROL(PLAYER_ID(), true, 0);
		// Clear screen effects as well
		ANIMPOSTFX_STOP_ALL();
		POP_TIMECYCLE_MODIFIER();

		if (m_ClearEffectsState == ClearEffectsState::AllRestartPermanent)
			RegisterPermanentEffects();

		m_ClearEffectsState = ClearEffectsState::None;
	}

	for (auto threadId : m_PermanentEffects)
		EffectThreads::RunThread(threadId);

	float adjustedDeltaTime = deltaTime / 1000.f;

	int activeEffects       = 0;
	// Reverse order to ensure the first effects are removed if activeEffects > m_MaxRunningEffects
	for (auto it = SharedState.ActiveEffects.rbegin(); it != SharedState.ActiveEffects.rend();)
	{
		auto &activeEffect  = *it;
		bool isEffectPaused = EffectThreads::IsThreadPaused(activeEffect.ThreadId);

		activeEffect.Timer -=
		    (adjustedDeltaTime
		     / (!ComponentExists<MetaModifiers>() ? 1.f : GetComponent<MetaModifiers>()->EffectDurationModifier))
		    * (activeEffect.IsTimed
		           ? 1.f
		           : std::max(1.f, .5f * (activeEffects - EFFECT_NONTIMED_TIMER_SPEEDUP_MIN_EFFECTS + 3)));

		if (!EffectThreads::DoesThreadExist(activeEffect.ThreadId) || activeEffect.IsZombie)
		{
			activeEffect.IsZombie = true;
			if (activeEffect.IsTimed || activeEffect.Timer <= 0.f)
			{
				DEBUG_LOG("Discarding ActiveEffect " << activeEffect.Id.Id());
				it = static_cast<decltype(it)>(SharedState.ActiveEffects.erase(std::next(it).base()));
				continue;
			}
		}
		else if (!isEffectPaused)
		{
			OnPreRunEffect.Fire(activeEffect.Id);
			EffectThreads::RunThread(activeEffect.ThreadId);
			OnPostRunEffect.Fire(activeEffect.Id);

			if (!activeEffect.IsMeta)
				activeEffects++;
		}

		if (activeEffect.HideEffectName && EffectThreads::HasThreadOnStartExecuted(activeEffect.ThreadId))
			activeEffect.HideEffectName = false;

		auto effectSharedData = EffectThreads::GetThreadSharedData(activeEffect.ThreadId);
		if (effectSharedData)
		{
			effectSharedData->EffectCompletionPercentage =
			    activeEffect.Timer <= 0.f ? 1.f : 1.f - activeEffect.Timer / activeEffect.MaxTime;

			if (ComponentExists<EffectSoundManager>() && activeEffect.SoundId)
				GetComponent<EffectSoundManager>()->SetSoundOptions(activeEffect.SoundId,
				                                                    effectSharedData->EffectSoundPlayOptions);

			if (!effectSharedData->OverrideEffectName.empty())
			{
				activeEffect.FakeName = effectSharedData->OverrideEffectName;
				effectSharedData->OverrideEffectName.clear();
			}

			if (!effectSharedData->OverrideEffectId.empty())
			{
				if (g_EnabledEffects.contains(effectSharedData->OverrideEffectId))
				{
					auto &fakeEffect      = g_EnabledEffects.at(effectSharedData->OverrideEffectId);
					activeEffect.FakeName = !fakeEffect.HasCustomName() ? fakeEffect.Name : fakeEffect.CustomName;
					if (fakeEffect.IsMeta())
						activeEffect.FakeName += " (Meta)";
				}
				else
				{
					auto result = g_RegisteredEffectsMetadata.find(effectSharedData->OverrideEffectId);
					if (result != g_RegisteredEffectsMetadata.end())
						activeEffect.FakeName = result->second.Name;
				}

				effectSharedData->OverrideEffectId.clear();
			}
		}

		if (!activeEffect.IsZombie // Shouldn't ever occur since the ActiveEffect is removed if timer <= 0 above, but
		                           // just in case this check is moved in the future
		    && (activeEffect.Timer <= 0.f || (!activeEffect.IsMeta && activeEffects > m_MaxRunningEffects)))
		{
			if (!activeEffect.IsStopping)
			{
				DEBUG_LOG("Stopping effect " << activeEffect.Id.Id());
				EffectThreads::StopThread(activeEffect.ThreadId);
				activeEffect.IsStopping = true;
			}
			else if (activeEffect.Timer < -60.f)
			{
				// Effect took over 60 seconds to stop, forcibly stop it in a blocking manner
				LOG("Timeout reached, forcefully stopping effect " << activeEffect.Id.Id());
				EffectThreads::StopThreadImmediately(activeEffect.ThreadId);
			}
		}

		it++;
	}
}

void EffectDispatcher::UpdateMetaEffects(float deltaTime)
{
	if (!SharedState.MetaEffectsEnabled)
		return;

	SharedState.MetaEffectTimerPercentage += (float)deltaTime / SharedState.MetaEffectSpawnTime / 1000;

	if (SharedState.MetaEffectTimerPercentage >= 1.f)
	{
		SharedState.MetaEffectTimerPercentage = 0.f;

		std::vector<std::tuple<EffectIdentifier, EffectData *>> availableMetaEffects;

		float totalWeight = 0.f;
		for (const auto &effectData : GetFilteredEnabledEffects())
		{
			if (effectData->IsMeta() && !effectData->IsUtility() && !effectData->IsHidden())
			{
				totalWeight += effectData->GetEffectWeight();

				availableMetaEffects.push_back(std::make_tuple(effectData->Id, effectData));
			}
		}

		if (!availableMetaEffects.empty())
		{
			// TODO: Stop duplicating effect weight logic everywhere
			float chosen                           = g_Random.GetRandomFloat(0.f, totalWeight);

			totalWeight                            = 0.f;

			const EffectIdentifier *targetEffectId = nullptr;
			for (const auto &[effectId, effectData] : availableMetaEffects)
			{
				totalWeight += effectData->GetEffectWeight();

				if (!targetEffectId && chosen <= totalWeight)
					targetEffectId = &effectId;
			}

			if (targetEffectId)
			{
				// Increase weight of all meta effects (including ones with an unfulfilled condition)
				for (auto &[effectId, effectData] : g_EnabledEffects)
					if (effectData.IsMeta() && !effectData.IsUtility() && !effectData.IsHidden())
						effectData.Weight += effectData.WeightMult;

				_DispatchEffect(this,
				                { .Id = *targetEffectId, .Suffix = "(Meta)", .Flags = DispatchEffectFlag_NoAddToLog });
			}
		}
		else
		{
			SharedState.MetaEffectsEnabled        = false;
			SharedState.MetaEffectTimerPercentage = 0.f;
		}
	}
}

void EffectDispatcher::DrawEffectTexts()
{
	if (m_DisableDrawEffectTexts)
		return;

	float y             = GetEffectTopSpace();
	float effectSpacing = EFFECT_TEXT_INNER_SPACING_MAX;

	if (SharedState.ActiveEffects.size() > 0)
	{
		effectSpacing =
		    std::min(EFFECT_TEXT_INNER_SPACING_MAX,
		             std::max(EFFECT_TEXT_INNER_SPACING_MIN, (1.0f - y) / SharedState.ActiveEffects.size()));
	}

	for (const ActiveEffect &effect : SharedState.ActiveEffects)
	{
		if (effect.IsStopping)
			continue;

		const bool hasFake = !effect.FakeName.empty();

		// Temporary non-timed effects will have their entries removed already since their OnStop is called immediately
		if (g_EnabledEffects.contains(effect.Id))
		{
			auto &effectData = g_EnabledEffects.at(effect.Id);
			if ((effect.HideEffectName && !hasFake)
			    || ((ComponentExists<MetaModifiers>()
			         && (GetComponent<MetaModifiers>()->HideChaosUI || GetComponent<MetaModifiers>()->DisableChaos))
			        && !effectData.IsMeta() && !effectData.IsUtility() && !effectData.IsTemporary()
			        && !effectData.ShouldAlwaysShowName()))
			{
				continue;
			}
		}

		bool showTimer             = false;
		float completionPercentage = effect.Timer / effect.MaxTime;
		auto effectSharedData      = EffectThreads::GetThreadSharedData(effect.ThreadId);
		if (effectSharedData)
		{
			float newPercentage = effectSharedData->OverrideEffectCompletionPercentage;
			if (newPercentage > 0.f && newPercentage < 1.f)
			{
				completionPercentage = newPercentage;
				showTimer            = true;
			}
		}

		auto effectName = effect.Name;
		if (effect.HideEffectName && hasFake)
			effectName = effect.FakeName;

		auto color = m_TextColor;

		if (ComponentExists<MetaModifiers>())
		{
			auto colorOverride = GetComponent<MetaModifiers>()->EffectTextColorOverride;
			for (size_t i = 0; i < 3; i++)
				if (colorOverride[i] > 0 && colorOverride[i] < 256)
					color[i] = colorOverride[i];
		}

		if (ComponentExists<MetaModifiers>() && GetComponent<MetaModifiers>()->FlipChaosUI)
		{
			DrawScreenText(effectName, { .085f, y }, .47f, { color[0], color[1], color[2] }, true,
			               ScreenTextAdjust::Left, { .0f, .915f });
		}
		else
		{
			DrawScreenText(effectName, { .915f, y }, .47f, { color[0], color[1], color[2] }, true,
			               ScreenTextAdjust::Right, { .0f, .915f });
		}

		if (effect.IsTimed || showTimer)
		{
			color = m_EffectTimerColor;

			if (ComponentExists<MetaModifiers>())
			{
				auto colorOverride = GetComponent<MetaModifiers>()->EffectTimerColorOverride;
				for (size_t i = 0; i < 3; i++)
					if (colorOverride[i] > 0 && colorOverride[i] < 256)
						color[i] = colorOverride[i];
			}

			if (ComponentExists<MetaModifiers>() && GetComponent<MetaModifiers>()->FlipChaosUI)
			{
				DRAW_RECT(.04f, y + .0185f, .05f, .019f, 0, 0, 0, 127, false);
				DRAW_RECT(.04f, y + .0185f, .048f * (1.f - completionPercentage), .017f, color[0], color[1], color[2],
				          255, false);
			}
			else
			{
				DRAW_RECT(.96f, y + .0185f, .05f, .019f, 0, 0, 0, 127, false);
				DRAW_RECT(.96f, y + .0185f, .048f * completionPercentage, .017f, color[0], color[1], color[2], 255,
				          false);
			}
		}

		y += effectSpacing;
	}
}

void EffectDispatcher::DispatchEffect(const EffectIdentifier &effectId, DispatchEffectFlags dispatchEffectFlags,
                                      const std::string &suffix, const bool increment)
{
	if (increment)
		EffectDispatchCount++;

	SetLastEffectId(effectId.Id());

	EffectDispatchQueue.push({ .Id = effectId, .Suffix = suffix, .Flags = dispatchEffectFlags });
}

void EffectDispatcher::DispatchEffectForMeta(const EffectIdentifier &effectId, const bool increment)
{
	DispatchEffect(effectId, DispatchEffectFlag_None, {}, increment);
}

std::string EffectDispatcher::GetRandomEffectId() const
{
	if (!m_EnableNormalEffectDispatch)
		return {};

	std::unordered_map<EffectIdentifier, EffectData, EffectsIdentifierHasher> choosableEffects;
	for (const auto &[effectId, effectData] : g_EnabledEffects)
		if (!effectData.IsMeta() && !effectData.IsUtility() && !effectData.IsHidden())
			choosableEffects.emplace(effectId, effectData);

	float totalWeight = 0.f;
	for (const auto &[effectId, effectData] : choosableEffects)
		totalWeight += effectData.GetEffectWeight();

	if (totalWeight <= 0.f)
		return nullptr;

	float chosen = g_Random.GetRandomFloat(0.f, totalWeight);
	totalWeight  = 0.f;


	for (const auto &[effectId, effectData] : choosableEffects)
	{
		totalWeight += effectData.GetEffectWeight();
		if (chosen <= totalWeight)
		{
			return effectId.Id();
		}
	}

	return nullptr;
}

void EffectDispatcher::DispatchRandomEffect(DispatchEffectFlags dispatchEffectFlags, const std::string &suffix)
{
	auto const randomEffect = GetRandomEffectId();

	if (randomEffect.empty())
		return;

	DispatchEffect(randomEffect, dispatchEffectFlags, suffix);
}

void EffectDispatcher::ClearEffect(const EffectIdentifier &effectId)
{
	auto result = std::find_if(SharedState.ActiveEffects.begin(), SharedState.ActiveEffects.end(),
	                           [effectId](auto &activeEffect) { return activeEffect.Id == effectId; });
	if (result == SharedState.ActiveEffects.end())
		return;

	EffectThreads::StopThread(result->ThreadId);
	result->IsStopping = true;
}

void EffectDispatcher::ClearEffects(ClearEffectsFlags clearEffectFlags)
{
	m_ClearEffectsState = clearEffectFlags & ClearEffectsFlag_NoRestartPermanentEffects
	                        ? ClearEffectsState::All
	                        : ClearEffectsState::AllRestartPermanent;
}

void EffectDispatcher::ClearActiveEffects(const std::string &ignoreEffect)
{
	for (auto it = SharedState.ActiveEffects.begin(); it != SharedState.ActiveEffects.end(); it++)
	{
		auto &effect = *it;

		if (effect.IsMeta || effect.Timer <= 0.f || effect.Id == ignoreEffect)
			continue;

		EffectThreads::StopThread(effect.ThreadId);
		effect.IsStopping = true;
	}
}

void EffectDispatcher::ClearMostRecentEffect()
{
	for (auto it = SharedState.ActiveEffects.rbegin(); it != SharedState.ActiveEffects.rend(); it++)
	{
		auto &effect = *it;

		if (effect.IsMeta || effect.Timer <= 0.f)
			continue;

		EffectThreads::StopThread(effect.ThreadId);
		effect.IsStopping = true;

		break;
	}
}

float EffectDispatcher::GetRemainingTimeForEffect(const EffectIdentifier &effectId)
{
	auto result = std::find_if(SharedState.ActiveEffects.begin(), SharedState.ActiveEffects.end(),
	                           [effectId](ActiveEffect &activeEffect) { return activeEffect.Id == effectId; });
	if (result == SharedState.ActiveEffects.end())
		return 0.f;
	return result->Timer;
}

void EffectDispatcher::SetRemainingTimeForEffect(const EffectIdentifier &effectId, float value)
{
	auto result = std::find_if(SharedState.ActiveEffects.begin(), SharedState.ActiveEffects.end(),
	                           [effectId](ActiveEffect &activeEffect) { return activeEffect.Id == effectId; });
	if (result == SharedState.ActiveEffects.end())
		return;
	result->Timer = value;
}

std::vector<RegisteredEffect *> EffectDispatcher::GetRecentEffects(int distance, const std::string &ignoreEffect) const
{
	std::vector<RegisteredEffect *> effects;

	for (int i = SharedState.DispatchedEffectsLog.size() - 1; distance > 0 && i >= 0; i--)
	{
		auto effect = *std::next(SharedState.DispatchedEffectsLog.begin(), i);
		if ((!ignoreEffect.empty() && effect->GetId() == ignoreEffect)
		    || std::find(effects.begin(), effects.end(), effect) != effects.end())
		{
			continue;
		}

		effects.emplace_back(effect);
		distance--;
	}

	return effects;
}

void EffectDispatcher::Reset(ClearEffectsFlags clearEffectFlags)
{
	ClearEffects(clearEffectFlags);

	SharedState.MetaEffectsEnabled        = true;
	SharedState.MetaEffectTimerPercentage = 0.f;
}

float EffectDispatcher::GetEffectTopSpace()
{
	return EnableEffectTextExtraTopSpace ? EFFECT_TEXT_TOP_SPACING_EXTRA : EFFECT_TEXT_TOP_SPACING;
}

void EffectDispatcher::RegisterPermanentEffects()
{
	auto registerEffect = [&](EffectIdentifier effectId)
	{
		auto *registeredEffect = GetRegisteredEffect(effectId);
		if (registeredEffect)
		{
			auto threadId = EffectThreads::CreateThread(registeredEffect);
			m_PermanentEffects.push_back(threadId);
		}
	};

	if (g_OptionsManager.GetConfigValue({ "Australia" }, false))
		registerEffect({ "player_flip_camera" });

	for (const auto &[effectId, effectData] : g_EnabledEffects)
	{
		if (effectData.TimedType == EffectTimedType::Permanent)
		{
			// Always run permanent timed effects in background
			registerEffect(effectId);
		}
	}
}

bool EffectDispatcher::IsClearingEffects() const
{
	return m_ClearEffectsState != ClearEffectsState::None;
}

std::string EffectDispatcher::GetLastEffectId() const
{
	return m_LastEffect;
}

void EffectDispatcher::SetLastEffectId(const std::string &effectId)
{
	auto const isNotRepeatEffect = (effectId.compare("misc_repeat_last_effect") != 0);

	if (isNotRepeatEffect)
		m_LastEffect = effectId;
}
