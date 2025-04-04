#include <stdafx.h>

#include "HelpTextQueue.h"

#include "Memory/Hooks/GetLabelTextHook.h"

void HelpTextQueue::DisplayLabel(std::string label, std::uint8_t durationSecs)
{
	if (durationSecs == 0)
		return;

	m_HelpTextQueue.emplace(label, durationSecs);
}

void HelpTextQueue::OnModPauseCleanup()
{
	Hooks::ClearCustomLabels();
}

void HelpTextQueue::OnRun()
{
	if (m_HelpTextQueue.empty())
		return;

	auto &helpText = m_HelpTextQueue.front();

	auto text = helpText.Text;

	if (!IS_HELP_MESSAGE_BEING_DISPLAYED()
	    || (BEGIN_TEXT_COMMAND_IS_THIS_HELP_MESSAGE_BEING_DISPLAYED(text.data()),
	        END_TEXT_COMMAND_IS_THIS_HELP_MESSAGE_BEING_DISPLAYED(0)))
	{
		BEGIN_TEXT_COMMAND_DISPLAY_HELP(text.data());
		END_TEXT_COMMAND_DISPLAY_HELP(0, false, false, -1);

		if ((helpText.TimerSecs -= GET_FRAME_TIME()) < 0.f)
			m_HelpTextQueue.pop();
	}
}