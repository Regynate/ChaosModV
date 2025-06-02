#pragma once

#include "Components/Component.h"

#include <IXWebSocket/IXWebSocket.h>

class DispatchSocket : public Component
{
	ix::WebSocket m_Socket;

  public:
	DispatchSocket();

    virtual void OnModPauseCleanup() override;
};