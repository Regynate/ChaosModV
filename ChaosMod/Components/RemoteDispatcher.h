#pragma once

#include "Component.h"
#include "EffectDispatcher.h"
#include "SplashTexts.h"


class RemoteDispatcher : public Component
{
  private:
	HANDLE m_hPipeHandle              = INVALID_HANDLE_VALUE;
  protected:
	RemoteDispatcher();
	virtual ~RemoteDispatcher() override;

  public:
	virtual void OnModPauseCleanup() override;
	virtual void OnRun() override;

	template <class T>
	requires std::is_base_of_v<Component, T>
	friend struct ComponentHolder;
};