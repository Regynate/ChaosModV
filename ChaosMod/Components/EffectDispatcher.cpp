#include <stdafx.h>

#include "EffectDispatcher.h"
#include "Mp3Manager.h"

#include "Components/MetaModifiers.h"

#include "Effects/EEffectCategory.h"
#include "Effects/MetaModifiers.h"

#include "Util/OptionsManager.h"
#include "Util/Random.h"
#include "Util/Text.h"

#define EFFECT_TEXT_INNER_SPACING_MIN .030f
#define EFFECT_TEXT_INNER_SPACING_MAX .075f
#define EFFECT_TEXT_TOP_SPACING .2f
#define EFFECT_TEXT_TOP_SPACING_EXTRA .35f

static void _DispatchEffect(EffectDispatcher *effectDispatcher, const EffectDispatcher::EffectDispatchEntry &entry)
{
	EffectData &effectData = g_EnabledEffects.at(entry.Identifier);
	if (effectData.TimedType == EffectTimedType::Permanent)
	{
		return;
	}

	if (!effectDispatcher->OnPreDispatchEffect.Fire(entry.Identifier))
	{
		return;
	}

	LOG("Dispatching effect \"" << effectData.Name << "\"");

	// Increase weight for all effects first
	for (auto &[effectId, effectData] : g_EnabledEffects)
	{
		if (!effectData.IsMeta())
		{
			effectData.Weight += effectData.WeightMult;
		}
	}

	// Reset weight of this effect (or every effect in group) to reduce chance of same effect (group) happening multiple
	// times in a row
	if (effectData.GroupType.empty())
	{
		effectData.Weight = effectData.WeightMult;
	}
	else
	{
		for (auto &[effectId, effectData] : g_EnabledEffects)
		{
			if (effectData.GroupType == effectData.GroupType)
			{
				effectData.Weight = effectData.WeightMult;
			}
		}
	}

	// Check if timed effect already is active, reset timer if so
	// Also check for incompatible effects
	bool alreadyExists          = false;

	const auto &incompatibleIds = effectData.IncompatibleIds;

	for (auto it = effectDispatcher->SharedState.ActiveEffects.begin();
	     it != effectDispatcher->SharedState.ActiveEffects.end();)
	{
		auto &activeEffect     = *it;
		auto &activeEffectData = g_EnabledEffects.at(activeEffect.Identifier);

		if (activeEffect.Identifier == entry.Identifier)
		{
			if (effectData.TimedType != EffectTimedType::NotTimed)
			{
				alreadyExists      = true;
				activeEffect.Timer = activeEffect.MaxTime;
			}
			else
			{
				EffectThreads::StopThreadImmediately(activeEffect.ThreadId);
				it = effectDispatcher->SharedState.ActiveEffects.erase(it);
			}

			break;
		}

		bool found = false;
		if (std::find(incompatibleIds.begin(), incompatibleIds.end(), activeEffectData.Id) != incompatibleIds.end())
		{
			found = true;
		}

		// Check if current effect is either the same effect category or marked as incompatible in active effect
		if (!found)
		{
			const auto &activeIncompatibleIds = activeEffectData.IncompatibleIds;
			if ((effectData.EffectCategory != EffectCategory::None
			     && effectData.EffectCategory == activeEffectData.EffectCategory)
			    || std::find(activeIncompatibleIds.begin(), activeIncompatibleIds.end(), effectData.Id)
			           != activeIncompatibleIds.end())
			{
				found = true;
			}
		}

		if (found)
		{
			EffectThreads::StopThread(activeEffect.ThreadId);
			activeEffect.IsStopping = true;
		}

		it++;
	}

	if (!alreadyExists)
	{
		RegisteredEffect *registeredEffect = GetRegisteredEffect(entry.Identifier);

		if (registeredEffect)
		{
			std::ostringstream effectName;
			effectName << (effectData.HasCustomName() ? effectData.CustomName : effectData.Name);

			if (!entry.Suffix.empty())
			{
				effectName << " " << entry.Suffix;
			}

			if ((ComponentExists<MetaModifiers>() && !GetComponent<MetaModifiers>()->HideChaosUI)
			    && ComponentExists<Mp3Manager>())
			{
				// Play global sound (if one exists)
				// HACK: Force no global sound for "Fake Crash"
				if (entry.Identifier.GetEffectId() != "misc_fakecrash")
				{
					GetComponent<Mp3Manager>()->PlayChaosSoundFile("global_effectdispatch");
				}

				// Play a sound if corresponding .mp3 file exists
				GetComponent<Mp3Manager>()->PlayChaosSoundFile(effectData.Id);
			}

			int effectDuration;
			switch (effectData.TimedType)
			{
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
				effectDuration = -1;
				break;
			}

			effectDispatcher->SharedState.ActiveEffects.emplace_back(entry.Identifier, registeredEffect,
			                                                         effectName.str(), effectData, effectDuration);

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

	effectDispatcher->OnPostDispatchEffect.Fire(entry.Identifier);
}

static void _OnRunEffects(LPVOID data)
{
	auto effectDispatcher = reinterpret_cast<EffectDispatcher *>(data);
	while (true)
	{
		auto currentUpdateTime = GetTickCount64();
		int deltaTime          = currentUpdateTime - effectDispatcher->Timer;

		// the game was paused
		if (deltaTime > 1000)
		{
			deltaTime = 0;
		}

		while (!effectDispatcher->EffectDispatchQueue.empty())
		{
			_DispatchEffect(effectDispatcher, effectDispatcher->EffectDispatchQueue.front());
			effectDispatcher->EffectDispatchQueue.pop();
		}

		effectDispatcher->UpdateEffects(deltaTime);

		if (!effectDispatcher->PauseTimer)
		{
			effectDispatcher->UpdateMetaEffects(deltaTime);
		}

		SwitchToFiber(g_MainThread);
	}
}

EffectDispatcher::EffectDispatcher(const std::array<BYTE, 3> &timerColor, const std::array<BYTE, 3> &textColor,
                                   const std::array<BYTE, 3> &effectTimerColor)
    : Component()
{
	m_TimerColor          = timerColor;
	m_TextColor           = textColor;
	m_EffectTimerColor    = effectTimerColor;

	m_DisableDrawTimerBar = g_OptionsManager.GetConfigValue({ "DisableTimerBarDraw" }, OPTION_DEFAULT_NO_EFFECT_BAR);
	m_DisableDrawEffectTexts =
	    g_OptionsManager.GetConfigValue({ "DisableEffectTextDraw" }, OPTION_DEFAULT_NO_TEXT_DRAW);

	m_EffectSpawnTime = g_OptionsManager.GetConfigValue({ "NewEffectSpawnTime" }, OPTION_DEFAULT_EFFECT_SPAWN_TIME);
	SharedState.EffectTimedDur = g_OptionsManager.GetConfigValue({ "EffectTimedDur" }, OPTION_DEFAULT_EFFECT_TIMED_DUR);
	SharedState.EffectTimedShortDur =
	    g_OptionsManager.GetConfigValue({ "EffectTimedShortDur" }, OPTION_DEFAULT_EFFECT_SHORT_TIMED_DUR);

	m_usMetaEffectSpawnTime =
	    g_OptionsManager.GetConfigValue<int>("NewMetaEffectSpawnTime", OPTION_DEFAULT_EFFECT_META_SPAWN_TIME);
	m_usMetaEffectTimedDur =
	    g_OptionsManager.GetConfigValue<int>("MetaEffectDur", OPTION_DEFAULT_EFFECT_META_TIMED_DUR);
	m_usMetaEffectShortDur =
	    g_OptionsManager.GetConfigValue<int>("MetaShortEffectDur", OPTION_DEFAULT_EFFECT_META_SHORT_TIMED_DUR);

	m_DistanceChaosState.EnableDistanceBasedEffectDispatch = g_OptionsManager.GetConfigValue(
	    { "EnableDistanceBasedEffectDispatch" }, OPTION_DEFAULT_DISTANCE_BASED_DISPATCH_ENABLED);
	m_DistanceChaosState.DistanceToActivateEffect =
	    g_OptionsManager.GetConfigValue({ "DistanceToActivateEffect" }, OPTION_DEFAULT_EFFECT_SPAWN_DISTANCE);
	m_DistanceChaosState.DistanceType = static_cast<DistanceChaosState::TravelledDistanceType>(
	    g_OptionsManager.GetConfigValue({ "DistanceType" }, OPTION_DEFAULT_DISTANCE_TYPE));

	m_MaxRunningEffects =
	    g_OptionsManager.GetConfigValue({ "MaxParallelRunningEffects" }, OPTION_DEFAULT_MAX_RUNNING_PARALLEL_EFFECTS);

	m_bEnableTwitchVoting =
	    g_OptionsManager.GetTwitchValue<bool>("EnableTwitchVoting", OPTION_DEFAULT_TWITCH_VOTING_ENABLED);

	m_eTwitchOverlayMode = static_cast<ETwitchOverlayMode>(
	    g_OptionsManager.GetTwitchValue<int>("TwitchVotingOverlayMode", OPTION_DEFAULT_TWITCH_OVERLAY_MODE));

	Reset(bSuspended);
}

void EffectDispatcher::OnModPauseCleanup()
{
	ClearEffects();
}

void EffectDispatcher::OnRun()
{
	auto currentUpdateTime = GetTickCount64();
	int deltaTime          = currentUpdateTime - Timer;

	// the game was paused
	if (deltaTime > 1000)
	{
		deltaTime = 0;
	}

	if (!PauseTimer && !m_DistanceChaosState.EnableDistanceBasedEffectDispatch)
	{
		UpdateTimer(deltaTime);
	}

	if (!PauseTimer && m_DistanceChaosState.EnableDistanceBasedEffectDispatch)
	{
		UpdateTravelledDistance();
	}

	DrawTimerBar();

	if (g_EffectDispatcherThread)
	{
		SwitchToFiber(g_EffectDispatcherThread);
	}

	DrawTimerBar();
	DrawEffectTexts();

	Timer = currentUpdateTime;
}

void EffectDispatcher::UpdateTravelledDistance()
{
	Ped player       = PLAYER_PED_ID();
	Vector3 position = GET_ENTITY_COORDS(player, false);

	if (IS_ENTITY_DEAD(player, false))
	{
		m_DeadFlag = true;
		return;
	}

	if (m_DeadFlag)
	{
		m_DeadFlag                         = false;
		m_DistanceChaosState.SavedPosition = GET_ENTITY_COORDS(player, false);
		return;
	}

	float distance =
	    GET_DISTANCE_BETWEEN_COORDS(position.x, position.y, position.z, m_DistanceChaosState.SavedPosition.x,
	                                m_DistanceChaosState.SavedPosition.y, m_DistanceChaosState.SavedPosition.z, true);

	if (m_DistanceChaosState.DistanceType == DistanceChaosState::TravelledDistanceType::Displacement)
	{
		if (distance >= m_DistanceChaosState.DistanceToActivateEffect)
		{
			if (DispatchEffectsOnTimer)
			{
				DispatchRandomEffect();

				if (ComponentExists<MetaModifiers>())
				{
					for (int i = 0; i < GetComponent<MetaModifiers>()->AdditionalEffectsToDispatch; i++)
					{
						DispatchRandomEffect();
					}
				}
			}

			m_DistanceChaosState.SavedPosition = position;
		}

		m_TimerPercentage = distance
		                  * (ComponentExists<MetaModifiers>() ? GetComponent<MetaModifiers>()->TimerSpeedModifier : 1.f)
		                  / m_DistanceChaosState.DistanceToActivateEffect;
	}
	else if (m_DistanceChaosState.DistanceType == DistanceChaosState::TravelledDistanceType::Distance)
	{
		m_DistanceChaosState.SavedPosition = position;
		m_TimerPercentage +=
		    distance * (ComponentExists<MetaModifiers>() ? GetComponent<MetaModifiers>()->TimerSpeedModifier : 1.f)
		    / m_DistanceChaosState.DistanceToActivateEffect;

		if (m_TimerPercentage >= 1.f && DispatchEffectsOnTimer)
		{
			DispatchRandomEffect();

			if (ComponentExists<MetaModifiers>())
			{
				for (int i = 0; i < GetComponent<MetaModifiers>()->AdditionalEffectsToDispatch; i++)
				{
					DispatchRandomEffect();
				}
			}

			m_TimerPercentage = 0;
		}
	}
}

void EffectDispatcher::UpdateTimer(int deltaTime)
{
	if (!m_EnableNormalEffectDispatch
	    || (ComponentExists<MetaModifiers>() && GetComponent<MetaModifiers>()->DisableChaos))
	{
		return;
	}

	m_TimerPercentage += deltaTime
	                   * (ComponentExists<MetaModifiers>() ? GetComponent<MetaModifiers>()->TimerSpeedModifier : 1.f)
	                   / m_EffectSpawnTime / 1000;

	if (m_TimerPercentage >= 1.f && DispatchEffectsOnTimer)
	{
		DispatchRandomEffect();

		if (ComponentExists<MetaModifiers>())
		{
			for (int i = 0; i < GetComponent<MetaModifiers>()->AdditionalEffectsToDispatch; i++)
			{
				DispatchRandomEffect();
			}
		}

		m_TimerPercentage = 0.f;
	}
}

void EffectDispatcher::UpdateEffects(int deltaTime)
{
	EffectThreads::RunThreads();

	// Don't continue if there are no enabled effects
	if (g_dictEnabledEffects.empty())
	{
		return;
	}

	for (ActiveEffect &effect : m_rgActiveEffects)
	{
		if (effect.m_bHideText && EffectThreads::HasThreadOnStartExecuted(effect.m_ullThreadId))
		{
			effect.m_bHideText = false;
		}
	}

	float fDeltaTime               = (float)iDeltaTime / 1000;

	int maxEffects =
	    std::min((int)(floor((1.0f - GetEffectTopSpace()) / EFFECT_TEXT_INNER_SPACING_MIN) - 1), m_MaxRunningEffects);
	int effectCountToCheckCleaning = 3;
	for (auto it = SharedState.ActiveEffects.begin(); it != SharedState.ActiveEffects.end();)
	{
		auto &effect = *it;

		if (EffectThreads::DoesThreadExist(effect.ThreadId) && !EffectThreads::IsThreadPaused(effect.ThreadId))
		{
			OnPreRunEffect.Fire(effect.Identifier);
			EffectThreads::RunThread(effect.ThreadId);
			OnPostRunEffect.Fire(effect.Identifier);
		}

		if (effect.HideEffectName && EffectThreads::HasThreadOnStartExecuted(effect.ThreadId))
		{
			effect.HideEffectName = false;
		}

		bool isTimed = false;
		bool isMeta  = false;
		// Temporary non-timed effects will have their entries removed already since their OnStop is called immediately
		if (g_EnabledEffects.contains(effect.Identifier))
		{
			auto &effectData = g_EnabledEffects.at(effect.Identifier);
			isTimed          = effectData.TimedType != EffectTimedType::NotTimed;
			isMeta           = effectData.IsMeta();
		}

		if (effect.MaxTime > 0)
		{
			if (isMeta)
			{
				effect.Timer -= adjustedDeltaTime;
			}
			else
			{
				effect.m_fTimer -= fDeltaTime / MetaModifiers::m_fEffectDurationModifier;
			}
		}
		else
		{
			float t = SharedState.EffectTimedDur, m = maxEffects, n = effectCountToCheckCleaning;
			// ensure effects stay on screen for at least 5 seconds
			effect.Timer += adjustedDeltaTime / t
			              * (1.f + (t / 5 - 1) * std::max(0.f, SharedState.ActiveEffects.size() - n) / (m - n));
		}

		if (effect.IsStopping)
		{
			EffectThreads::StopThreadImmediately(effect.ThreadId);
			it = SharedState.ActiveEffects.erase(it);
		}
		else
		{
			if (effect.MaxTime > 0 && effect.Timer <= 0
			    || !isTimed && (SharedState.ActiveEffects.size() > maxEffects || effect.Timer >= 0.f)
			           && !effect.IsStopping)
			{
				EffectThreads::StopThread(effect.ThreadId);
				effect.IsStopping = true;
			}

		it++;
	}
}

void EffectDispatcher::UpdateMetaEffects(int iDeltaTime)
{
	if (!m_bMetaEffectsEnabled)
	{
		return;
	}

	m_fMetaEffectTimerPercentage += (float)iDeltaTime / m_usMetaEffectSpawnTime / 1000;

	if (m_fMetaEffectTimerPercentage >= 1.f)
	{
		m_fMetaEffectTimerPercentage = 0.f;

		std::vector<std::tuple<EffectIdentifier, EffectData *>> availableMetaEffects;

		float totalWeight = 0.f;
		for (auto &[effectId, effectData] : g_dictEnabledEffects)
		{
			if (effectData.IsMeta() && !effectData.IsUtility() && !effectData.IsHidden())
			{
				totalWeight += GetEffectWeight(effectData);

				availableMetaEffects.push_back(std::make_tuple(effectId, &effectData));
			}
		}

		if (!availableMetaEffects.empty())
		{
			// TODO: Stop duplicating effect weight logic everywhere
			float chosen                                   = g_Random.GetRandomFloat(0.f, totalWeight);

			totalWeight                                    = 0.f;

			const EffectIdentifier *targetEffectIdentifier = nullptr;
			for (const auto &[effectIdentifier, effectData] : availableMetaEffects)
			{
				totalWeight += GetEffectWeight(*effectData);

				effectData->Weight += effectData->WeightMult;

				if (!targetEffectIdentifier && chosen <= totalWeight)
				{
					targetEffectIdentifier = &effectIdentifier;
				}
			}

			if (targetEffectIdentifier)
			{
				DispatchEffect(*targetEffectIdentifier, "(Meta)", false);
			}
		}
		else
		{
			m_bMetaEffectsEnabled        = false;
			m_fMetaEffectTimerPercentage = 0.f;
		}
	}
}

void EffectDispatcher::DrawTimerBar()
{
	if (!m_EnableNormalEffectDispatch || m_DisableDrawTimerBar
	    || (ComponentExists<MetaModifiers>()
	        && (GetComponent<MetaModifiers>()->HideChaosUI || GetComponent<MetaModifiers>()->DisableChaos)))
	{
		return;
	}

	float percentage =
	    FakeTimerBarPercentage > 0.f && FakeTimerBarPercentage <= 1.f ? FakeTimerBarPercentage : m_TimerPercentage;

	// New Effect Bar
	DRAW_RECT(.5f, .01f, 1.f, .021f, 0, 0, 0, 127, false);

	if (ComponentExists<MetaModifiers>() && GetComponent<MetaModifiers>()->FlipChaosUI)
	{
		DRAW_RECT(1.f - percentage * .5f, .01f, percentage, .018f, m_TimerColor[0], m_TimerColor[1], m_TimerColor[2],
		          255, false);
	}
	else
	{
		DRAW_RECT(percentage * .5f, .01f, percentage, .018f, m_TimerColor[0], m_TimerColor[1], m_TimerColor[2], 255,
		          false);
	}
}

void EffectDispatcher::DrawEffectTexts()
{
	if (m_bDisableDrawEffectTexts)
	{
		return;
	}

	float fPosY         = GetEffectTopSpace();
	float effectSpacing = m_fEffectsInnerSpacingMax;

	if (m_rgActiveEffects.size() > 0)
	{
		effectSpacing = std::min(m_fEffectsInnerSpacingMax,
		                         std::max(m_fEffectsInnerSpacingMin, (1.0f - fPosY) / m_rgActiveEffects.size()));
	}

	for (const ActiveEffect &effect : m_rgActiveEffects)
	{
		const bool hasFake = !effect.FakeName.empty();

		// Temporary non-timed effects will have their entries removed already since their OnStop is called immediately
		if (g_dictEnabledEffects.contains(effect.m_EffectIdentifier))
		{
			auto &effectData = g_dictEnabledEffects.at(effect.m_EffectIdentifier);

			if ((effect.m_bHideText && !bHasFake)
			    || (MetaModifiers::m_bHideChaosUI && !effectData.IsMeta() && !effectData.IsUtility()
			        && !effectData.IsTemporary())
			    || (MetaModifiers::m_bDisableChaos && !effectData.IsMeta() && !effectData.IsUtility()
			        && !effectData.IsTemporary()))
			{
				continue;
			}
		}

		std::string name = effect.m_szFakeName;

		if (!effect.m_bHideText || !bHasFake)
		{
			name = effect.m_szName;
		}

		if (MetaModifiers::m_bFlipChaosUI)
		{
			DrawScreenText(name, { .085f, fPosY }, .47f, { m_rgTextColor[0], m_rgTextColor[1], m_rgTextColor[2] }, true,
			               EScreenTextAdjust::Left, { .0f, .915f });
		}
		else
		{
			DrawScreenText(name, { .915f, fPosY }, .47f, { m_rgTextColor[0], m_rgTextColor[1], m_rgTextColor[2] }, true,
			               EScreenTextAdjust::Right, { .0f, .915f });
		}

		if (effect.MaxTime > 0)
		{
			if (MetaModifiers::m_bFlipChaosUI)
			{
				DRAW_RECT(.04f, fPosY + .0185f, .05f, .019f, 0, 0, 0, 127, false);
				DRAW_RECT(.04f, fPosY + .0185f, .048f * (1.f - (effect.m_fTimer / effect.m_fMaxTime)), .017f,
				          m_rgEffectTimerColor[0], m_rgEffectTimerColor[1], m_rgEffectTimerColor[2], 255, false);
			}
			else
			{
				DRAW_RECT(.96f, fPosY + .0185f, .05f, .019f, 0, 0, 0, 127, false);
				DRAW_RECT(.96f, fPosY + .0185f, .048f * effect.m_fTimer / effect.m_fMaxTime, .017f,
				          m_rgEffectTimerColor[0], m_rgEffectTimerColor[1], m_rgEffectTimerColor[2], 255, false);
			}
		}

		fPosY += effectSpacing;
	}
}

bool EffectDispatcher::ShouldDispatchEffectNow() const
{
	return GetRemainingTimerTime() <= 0;
}

int EffectDispatcher::GetRemainingTimerTime() const
{
	return std::ceil(m_EffectSpawnTime
	                 / (ComponentExists<MetaModifiers>() ? GetComponent<MetaModifiers>()->TimerSpeedModifier : 1.f)
	                 * (1 - m_TimerPercentage));
}

	if (ComponentExists<CrossingChallenge>())
	{
		GetComponent<CrossingChallenge>()->IncrementEffects();
	}
}

void EffectDispatcher::DispatchRandomEffect(const char *szSuffix)
{
	if (!m_bEnableNormalEffectDispatch)
	{
		return;
	}

	std::unordered_map<EffectIdentifier, EffectData, EffectsIdentifierHasher> choosableEffects;
	for (const auto &[effectIdentifier, effectData] : g_dictEnabledEffects)
	{
		if (!effectData.IsMeta() && !effectData.IsUtility() && !effectData.IsHidden())
		{
			choosableEffects.emplace(effectIdentifier, effectData);
		}
	}

	float fTotalWeight = 0.f;
	for (const auto &[effectIdentifier, effectData] : choosableEffects)
	{
		fTotalWeight += GetEffectWeight(effectData);
	}

	float fChosen                                   = g_Random.GetRandomFloat(0.f, fTotalWeight);

	fTotalWeight                                    = 0.f;

	const EffectIdentifier *pTargetEffectIdentifier = nullptr;
	for (const auto &[effectIdentifier, effectData] : choosableEffects)
	{
		fTotalWeight += GetEffectWeight(effectData);

		if (fChosen <= fTotalWeight)
		{
			pTargetEffectIdentifier = &effectIdentifier;

			break;
		}
	}

	if (pTargetEffectIdentifier)
	{
		DispatchEffect(*pTargetEffectIdentifier, szSuffix);
	}
}

void EffectDispatcher::ClearEffect(const EffectIdentifier &effectId)
{
	auto result = std::find_if(m_rgActiveEffects.begin(), m_rgActiveEffects.end(),
	                           [effectId](auto &activeEffect) { return activeEffect.m_EffectIdentifier == effectId; });
	if (result == m_rgActiveEffects.end())
	{
		return;
	}

	EffectThreads::StopThreadBlocking(result->m_ullThreadId);
	m_rgActiveEffects.erase(result);
}

void EffectDispatcher::ClearEffects(bool bIncludePermanent)
{
	EffectThreads::StopThreads();

	if (bIncludePermanent)
	{
		m_rgPermanentEffects.clear();
	}

	m_rgActiveEffects.clear();
	m_rgDispatchedEffectsLog.clear();
}

void EffectDispatcher::ClearActiveEffects()
{
	for (auto it = SharedState.ActiveEffects.begin(); it != SharedState.ActiveEffects.end();)
	{
		ActiveEffect &effect = *it;

		if (effect.Identifier != exclude)
		{
			EffectThreads::StopThread(effect.ThreadId);
			effect.IsStopping = true;
		}

		it++;
	}
}

void EffectDispatcher::ClearMostRecentEffect()
{
	if (!SharedState.ActiveEffects.empty())
	{
		ActiveEffect &mostRecentEffect = SharedState.ActiveEffects[SharedState.ActiveEffects.size() - 1];

		if (mostRecentEffect.Timer > 0)
		{
			EffectThreads::StopThread(mostRecentEffect.ThreadId);
			mostRecentEffect.IsStopping = true;
		}

		EffectThreads::StopThread(effect.ThreadId);
		effect.IsStopping = true;

		break;
	}
}

std::vector<RegisteredEffect *> EffectDispatcher::GetRecentEffects(int distance, std::string_view ignoreEffect) const
{
	std::vector<RegisteredEffect *> effects;

	for (int i = m_rgDispatchedEffectsLog.size() - 1; distance > 0 && i >= 0; i--)
	{
		auto effect = *std::next(m_rgDispatchedEffectsLog.begin(), i);
		if ((!ignoreEffect.empty() && effect->GetIndentifier().GetEffectId() == ignoreEffect)
		    || std::find(effects.begin(), effects.end(), effect) != effects.end())
		{
			continue;
		}

		effects.emplace_back(effect);
		distance--;
	}

	return effects;
}

void EffectDispatcher::Reset(bool bSuspended)
{
	ClearEffects(clearEffectFlags);
	ResetTimer();

	if (!bSuspended)
	{
		for (const auto &[effectIdentifier, effectData] : g_dictEnabledEffects)
		{
			if (effectData.TimedType == EEffectTimedType::Permanent)
			{
				// Always run permanent timed effects in background
				RegisteredEffect *pRegisteredEffect = GetRegisteredEffect(effectIdentifier);

void EffectDispatcher::ResetTimer()
{
	m_TimerPercentage = 0.f;
	Timer             = GetTickCount64();
}

float EffectDispatcher::GetEffectTopSpace()
{
	return EnableEffectTextExtraTopSpace ? EFFECT_TEXT_TOP_SPACING_EXTRA : EFFECT_TEXT_TOP_SPACING;
}

	m_bSuspended = bSuspended;
}

void EffectDispatcher::ResetTimer()
{
	m_fTimerPercentage = 0.f;
	m_ullTimer         = GetTickCount64();
}

float EffectDispatcher::GetEffectTopSpace()
{
	if (m_bEnableTwitchVoting
	    && (m_eTwitchOverlayMode == ETwitchOverlayMode::OverlayIngame
	        || m_eTwitchOverlayMode == ETwitchOverlayMode::OverlayOBS))
	{
		return m_fEffectsTopSpacingWithVoting;
	}
	return m_fEffectsTopSpacingDefault;
}

// (kolyaventuri): Forces the name of the provided effect to change, using any given string
void EffectDispatcher::OverrideEffectName(std::string_view effectId, const std::string &szOverrideName)
{
	for (auto &effect : m_rgActiveEffects)
	{
		if (effect.m_EffectIdentifier.GetEffectId() == effectId)
		{
			effect.m_szFakeName = szOverrideName;
		}
	}
}

// (kolyaventuri): Forces the name of the provided effect to change, using the defined name of another effect
void EffectDispatcher::OverrideEffectNameId(std::string_view effectId, std::string_view fakeEffectId)
{
	for (auto &effect : m_rgActiveEffects)
	{
		if (effect.m_EffectIdentifier.GetEffectId() == effectId)
		{
			auto effectIdentifier = EffectIdentifier(std::string(fakeEffectId));

			if (g_dictEnabledEffects.contains(effectIdentifier))
			{
				auto &fakeEffect    = g_dictEnabledEffects.at(effectIdentifier);
				effect.m_szFakeName = fakeEffect.HasCustomName() ? fakeEffect.CustomName : fakeEffect.Name;
			}
			else
			{
				auto result = g_dictEffectsMap.find(fakeEffectId);
				if (result != g_dictEffectsMap.end())
				{
					effect.m_szFakeName = result->second.Name;
				}
			}
		}
	}
}
