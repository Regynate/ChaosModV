#pragma once

#include <string>

#include "Userstate.h"

struct ChatMessageDeletion
{
	std::string m_MessageId;
	std::string m_Message;

	ChatMessageDeletion(std::string id, std::string message) : m_MessageId(id), m_Message(message)
	{
	}
};
