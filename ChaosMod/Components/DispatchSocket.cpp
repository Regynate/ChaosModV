#include "DispatchSocket.h"

DispatchSocket::DispatchSocket()
{
	m_Socket.setUrl("wss://localhost/");

	m_Socket.setOnMessageCallback(
	    [](const ix::WebSocketMessagePtr &msg)
	    {
		    if (msg->type == ix::WebSocketMessageType::Message)
		    {
			    LOG("received message: " << msg->str);
		    }
		    else if (msg->type == ix::WebSocketMessageType::Open)
		    {
			    LOG("Connection established");
		    }
		    else if (msg->type == ix::WebSocketMessageType::Error)
		    {
			    // Maybe SSL is not configured properly
			    LOG("Connection error: " << msg->errorInfo.reason);
		    }
	    });

	m_Socket.start();
}

void DispatchSocket::OnModPauseCleanup()
{
    m_Socket.stop();
}
