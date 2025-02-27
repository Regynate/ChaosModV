#pragma once

#include <array>
#include <string>

struct Userstate
{
	// todo: badges, emotes
	std::string m_Username;
	std::string m_DisplayName;
	std::string m_Userid;
	std::string m_ColorHex;
	// bool m_Mod;
	// bool m_Subscriber;

	Userstate(std::string username, std::string displayName, std::string userid, std::string color)
	    : m_Username(username), m_DisplayName(displayName), m_Userid(userid), m_ColorHex(color)
	{
	}
};
