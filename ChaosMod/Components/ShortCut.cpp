#include "stdafx.h"

#include "ShortCut.h"



void ShortCut::Run()
{
	if (m_bDispatchEffect && m_pChosenEffectIdentifier)
	{
		m_bDispatchEffect = false;

		g_pEffectDispatcher->DispatchEffect(*m_pChosenEffectIdentifier);
	}
}


void ShortCut::HandleInput(DWORD ulKey, bool bOnRepeat)
{

	if (bOnRepeat)
	{
		return;
	}

	switch (ulKey)
	{
	case VK_F13:
		this->DispatchShortcutEffect(EEffectType::EFFECT_ANGRY_JESUS);
		break;
	case VK_F14:
		this->DispatchShortcutEffect(EEffectType::EFFECT_WHALE_RAIN);
		break;
	case VK_F15:
		this->DispatchShortcutEffect(EEffectType::EFFECT_PLAYER_GRAVITY);
		break;
	case VK_F16:
		this->DispatchShortcutEffect(EEffectType::EFFECT_MISC_AIRSTRIKE);
		break;
	case VK_F17:
		this->DispatchShortcutEffect(EEffectType::EFFECT_SPAWN_FAN_GIRLS);
		break;
	case VK_F18:
		this->DispatchShortcutEffect(EEffectType::EFFECT_META_ADDITIONAL_EFFECTS);
		break;
	case VK_F19:
		this->DispatchShortcutEffect(EEffectType::EFFECT_SPAWN_ARMORED_KURUMA);
		break;
	case VK_F20:
		this->DispatchShortcutEffect(EEffectType::EFFECT_VEH_REPLACEVEHICLE);
		break;
	case VK_F21:
		this->DispatchShortcutEffect(EEffectType::EFFECT_RAPID_FIRE);
		break;
	case VK_F22:
		this->DispatchShortcutEffect(EEffectType::EFFECT_VEH_FLYING_CAR);
		break;
	case VK_F24:
		this->DispatchShortcutEffect(EEffectType::EFFECT_SNOW);
		break;
	}
}

void ShortCut::DispatchShortcutEffect(EEffectType type)
{
	EffectIdentifier identifier = EffectIdentifier(type);
	if (g_EnabledEffects.contains(identifier))
	{
		m_pChosenEffectIdentifier = std::make_unique<EffectIdentifier>(identifier);
		m_bDispatchEffect = true;
	}
}