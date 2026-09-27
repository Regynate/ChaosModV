#pragma once

#include "Util/OptionDefaults.h"
#include "Util/OptionsFile.h"

#include <string>
#include <vector>

class OptionsManager
{
  private:
	OptionsFile m_ConfigFile { { "chaosmod/configs/config.json", "chaosmod/configs/config.ini",
	                             "chaosmod/config.ini" } };
	OptionsFile m_TwitchFile { { "chaosmod/configs/voting.json", "chaosmod/configs/voting.ini",
	                             "chaosmod/configs/twitch.ini", "chaosmod/twitch.ini" } };

  public:
	void Reset()
	{
		m_ConfigFile.Reset();
		m_TwitchFile.Reset();
	}

	template <typename T> inline T GetConfigValue(const std::vector<std::string> &lookupKeys, T defaultValue = {})
	{
		return GetOptionValue(m_ConfigFile, lookupKeys, defaultValue);
	}

	template <typename T> inline T GetConfigValue(std::string_view lookupKey, T defaultValue = {})
	{
		return GetOptionValue(m_ConfigFile, { std::string(lookupKey) }, defaultValue);
	}

	template <typename T> inline T GetVotingValue(const std::vector<std::string> &lookupKeys, T defaultValue = {})
	{
		return GetOptionValue(m_TwitchFile, lookupKeys, defaultValue);
	}

	template <typename T> inline T GetVotingValue(std::string_view lookupKey, T defaultValue = {})
	{
		return GetOptionValue(m_TwitchFile, { std::string(lookupKey) }, defaultValue);
	}

  private:
	template <typename T>
	inline T GetOptionValue(const OptionsFile &optionsFile, const std::vector<std::string> &keys, T defaultValue = {})
	{
		return optionsFile.ReadValue(keys, defaultValue);
	}
};

inline OptionsManager g_OptionsManager;