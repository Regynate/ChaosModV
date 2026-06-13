#pragma once
#ifdef WITH_DEBUG_PANEL_SUPPORT

#include "Components/Component.h"
#include "Components/EffectDispatcher.h"

#include <ixwebsocket/IXWebSocketServer.h>

#include <json.hpp>

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <string_view>
#include <unordered_map>

class DebugSocket : public Component
{
  public:
	std::queue<std::function<void()>> m_DelegateQueue;
	std::mutex m_DelegateQueueMutex;

	bool m_IsProfiling = false;
	struct EffectTraceStats
	{
		std::uint64_t EntryTimestamp = 0;
		float TotalExecTime          = 0;
		float MaxExecTime            = 0;
		struct TraceEntry
		{
			std::uint64_t Timestamp = 0;
			float ExecTime          = 0;
		};
		std::list<TraceEntry> ExecTraces;
	};
	std::unordered_map<std::string, EffectTraceStats> m_EffectTraceStats;

	std::mutex m_MessageMutex;
	std::vector<nlohmann::json> m_Messages;

  private:
	std::unique_ptr<ix::WebSocketServer> m_Server;

	CHAOS_EVENT_LISTENER(EffectDispatcher::OnPreDispatchEffect) m_OnPreDispatchEffectListener;
	CHAOS_EVENT_LISTENER(EffectDispatcher::OnPreRunEffect) m_OnPreRunEffectListener;
	CHAOS_EVENT_LISTENER(EffectDispatcher::OnPostRunEffect) m_OnPostRunEffectListener;

	std::mutex m_CommitMessageMutex;
	std::vector<nlohmann::json> m_CommittedMessages;

  public:
	DebugSocket();

	virtual void OnModPauseCleanup(PauseCleanupFlags cleanupFlags = {}) override;
	virtual void OnRun() override;

  private:
	void Connect();

  public:
	void Close();

	void ScriptLog(std::string_view scriptName, std::string_view text);

	void Send(nlohmann::json json);
	void CommitMessages();
	std::vector<nlohmann::json> GetMessages();

	inline int GetClientCount()
	{
		return m_Server->getClients().size();
	}
};

#endif