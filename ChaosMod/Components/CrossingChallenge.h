#pragma once

#include <list>

#include "Component.h"
#include "Util/Text.h"
#include "Util/OptionsFile.h"

#define SPLASH_TEXT_DUR_SECS 10

class CrossingChallenge : public Component
{
  private:
	struct WeaponInfo
	{
		Hash hash;
		int ammo;
	};

	OptionsFile m_fConfigFile { "chaosmod\\crossing.ini" };

	int m_iHelpMessageTick          = -1;

	bool m_bCaptureStartFlag        = false;
	bool m_bCaptureEndFlag          = false;

	int m_bStartedState             = 0;

	bool m_bWaitingConfirm          = false;

	bool m_bStartEnabled            = false;
	Vector3 m_vStartLocation        = Vector3();
	Hash m_hStartVehicleHash        = 0;
	float m_fStartHeading           = 0.f;
	float m_fStartCameraHeading     = 0.f;
	Hash m_hStartWeatherType1       = 0;
	Hash m_hStartWeatherType2       = 0;
	float m_fStartWeatherPercent    = 0.f;
	int m_iClockHours               = 0;
	int m_iClockMinutes             = 0;
	int m_iClockSeconds             = 0;
	int m_iStartWeaponAmmo          = 0;
	std::vector<WeaponInfo> m_rgStartWeapons;

	bool m_bEndEnabled              = false;
	Vector3 m_vEndLocation          = Vector3();
	float m_fEndRadius              = 0.f;

	int m_hButtonsScaleformHandle   = 0;
	bool m_bButtonsScaleformLoading = false;

	int m_hPassedScaleformHandle    = 0;
	bool m_bPassedScaleformLoading  = false;
	int m_iPassedScaleformTick      = 0;
	bool m_bPassedTransitionStarted = false;

	Blip m_bStartBlip               = 0;
	Blip m_bEndBlip                 = 0;

	DWORD m_dwTickCount             = 0;
	DWORD m_dwLastTick              = 0;
	int m_iEffectsCount             = 0;
	bool m_bTimerStarted              = 0;

	void ShowHint(const std::string &text);

	void SetStartParams();
	void ControlRespawn();

	int m_iPassedState = 0;

	bool CheckEndReached();
	void ShowPassedScaleform();
	void ControlPassed();

	void SaveConfig();

	void CaptureStart();
	void CaptureEnd();
	void IncreaseEndRadius();
	void DecreaseEndRadius();
	bool IsEndValid();
	void ShowEndCylinder();
	void ShowHelpButtons();
	void ShowBlips();

	void ShowProgress();

  protected:
	CrossingChallenge();

  public:
	virtual void OnRun() override;
	virtual void OnModPauseCleanup() override;
	void HandleInput(DWORD ulKey, bool bOnRepeat);
	inline void IncrementEffects()
	{
		m_iEffectsCount++;
	}

	template <class T>
	requires std::is_base_of_v<Component, T>
	friend struct ComponentHolder;
};