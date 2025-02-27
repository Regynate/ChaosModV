#pragma once

#include "Userstate.h"

struct ChatUserBan
{
	std::string m_Username;
	std::string m_Userid;

	ChatUserBan(std::string username, std::string userid) : m_Username(username), m_Userid(userid)
	{
	}
};
