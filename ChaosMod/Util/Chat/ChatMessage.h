#pragma once

#include <string>

#include "Userstate.h"

struct ChatMessage
{
	std::string m_Id;
	std::string m_Message;
	Userstate m_Userstate;

	ChatMessage(std::string id, std::string message, Userstate userstate)
	    : m_Id(id), m_Message(message), m_Userstate(userstate)
	{
	}
};
