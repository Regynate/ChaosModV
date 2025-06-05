#pragma once

#include "Components/Component.h"

#include <IXWebSocket/IXWebSocket.h>

#include <json.hpp>

class DispatchSocket : public Component
{
	struct EffectEntry
	{
		std::string Id;
		std::string Name;
		int Index;
		bool Enabled;
	};

	std::vector<EffectEntry> m_Effects;

	std::unique_ptr<ix::WebSocket> m_Socket;

	bool m_ErrorSplashShown;

	void SendToSocket(const std::string &type, const std::string &nonce, const nlohmann::json &data);
	void HandleMessage(const std::string &message);
	void OnEffectDispatched(const EffectIdentifier &effectId, const std::string context);
	void OnEffectFailed(const EffectIdentifier &effectId, const std::string context);

  public:
	DispatchSocket();

	virtual void OnModPauseCleanup() override;
};