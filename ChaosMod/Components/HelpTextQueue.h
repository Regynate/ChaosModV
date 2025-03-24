#pragma once

#include "Components/Component.h"

#include <queue>

class HelpTextQueue : public Component
{
	struct HelpText
	{
		std::string Text;
		float TimerSecs;
	};
	std::queue<HelpText> m_HelpTextQueue;

  public:
	void DisplayLabel(std::string label, std::uint8_t durationSecs);

	virtual void OnModPauseCleanup() override;
	virtual void OnRun() override;
};