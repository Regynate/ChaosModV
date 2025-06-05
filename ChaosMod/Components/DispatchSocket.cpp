#include <stdafx.h>

#include "DispatchSocket.h"

#include "Components/EffectDispatcher.h"
#include "Components/SplashTexts.h"
#include "Effects/EnabledEffects.h"
#include "Util/OptionsManager.h"

using namespace nlohmann;

static CHAOS_EVENT_LISTENER(EffectDispatcher::OnPostDispatchEffect) onEffectDispatchListener;
static CHAOS_EVENT_LISTENER(EffectDispatcher::OnDispatchEffectFailed) onEffectDispatchFailListener;

static std::string trim(std::string s)
{
	auto not_space = [](unsigned char c)
	{
		return !std::isspace(c);
	};
	s.erase(std::ranges::find_if(s | std::views::reverse, not_space).base(), s.end());
	s.erase(s.begin(), std::ranges::find_if(s, not_space));

	return s;
}

void DispatchSocket::SendToSocket(const std::string &type, const std::string &nonce, const json &data)
{
	json message;
	message["type"]  = type;
	message["nonce"] = nonce;
	message["data"]  = data;

	m_Socket->send(message.dump());
}

static int StringCompare(std::string_view a, std::string_view b)
{
	for (size_t i = 0;; i++)
	{
		if (i == a.size())
			return i - b.size();
		else if (i == b.size())
			return a.size() - i;

		auto ai = std::toupper(a[i]);
		auto bi = std::toupper(b[i]);
		if (ai != bi)
			return bi - ai;
	}
}

void ShowErrorMessage(std::string_view message)
{
	std::wstring wStr = { message.begin(), message.end() };
	MessageBox(NULL, wStr.c_str(), L"ChaosModV Error", MB_OK | MB_ICONERROR);
}

void ShowErrorSplash(std::string_view message)
{
	if (ComponentExists<SplashTexts>())
			GetComponent<SplashTexts>()->ShowSplash(std::string(message), { .83f, .62f }, .6f, { 255, 100, 100 }, 3);
}

void DispatchSocket::HandleMessage(const std::string &message)
{
	json messageJson;
	try
	{
		messageJson = json::parse(message);
	}
	catch (json::parse_error &exception)
	{
		LOG("Received invalid message: " << exception.what());
		return;
	}

	const auto &type         = messageJson["type"];
	const auto &nonceObj     = messageJson["nonce"];

	const std::string &nonce = nonceObj.is_string() ? nonceObj : "";

	if (type == "trigger")
	{
		bool result = false;

		try
		{
			const auto &data   = messageJson["data"];
			const auto &effect = data["effect"];
			const auto &sender = data.contains("sender") ? data["sender"] : json();

			const auto query   = trim(effect.get<std::string>());

			std::string effectId;

			for (const auto &entry : m_Effects)
			{
				if (entry.Enabled
				    && (query == std::to_string(entry.Index) || !StringCompare(query, entry.Id)
				        || !StringCompare(query, entry.Name)))
				{
					effectId = entry.Id;
					break;
				}
			}

			if (!effectId.empty())
			{
				if (ComponentExists<EffectDispatcher>())
				{
					GetComponent<EffectDispatcher>()->DispatchEffect(
					    effectId, {}, sender.is_string() ? "(" + sender.get<std::string>() + ")" : "", nonce);
					result = true;
				}
			}
		}
		catch (json::exception &e)
		{
			LOG("Invalid message: " << e.what());
			result = false;
		}

		if (!result)
			OnEffectFailed(EffectIdentifier(), nonce);
	}
	else if (type == "get-effects")
	{
		json effectsArray;

		for (const auto &entry : m_Effects)
		{
			json effect;
			effect["index"]   = entry.Index;
			effect["id"]      = entry.Id;
			effect["name"]    = entry.Name;
			effect["enabled"] = entry.Enabled;
			effectsArray.push_back(effect);
		}

		json data;
		data["effects"] = effectsArray;

		SendToSocket("effects-list", nonce, data);
	}
	else if (type == "auth")
	{
		if (ComponentExists<SplashTexts>())
			GetComponent<SplashTexts>()->ShowSplash("Points redemption active", { .83f, .62f }, .6f, { 255, 100, 100 });
	}
	else if (type == "no-auth")
	{
		ShowErrorMessage("Channel point redemption unavailable! Check your token in the config app");
	}
}

void DispatchSocket::OnEffectDispatched(const EffectIdentifier &effectId, const std::string context)
{
	SendToSocket("trigger-success", context, {});
}

void DispatchSocket::OnEffectFailed(const EffectIdentifier &effectId, const std::string context)
{
	SendToSocket("trigger-error", context, {});
}

DispatchSocket::DispatchSocket()
{
	m_ErrorSplashShown       = false;

	const auto enabled = g_OptionsManager.GetVotingValue({ "EnableChannelPoints" }, false);
	const auto server  = g_OptionsManager.GetVotingValue({ "ChannelPointsServer" }, std::string("regynate.com"));
	const auto channel = g_OptionsManager.GetVotingValue({ "TwitchChannelName" }, std::string(""));
	const auto token   = g_OptionsManager.GetVotingValue({ "ChannelPointsToken" }, std::string(""));

	if (!enabled)
		return;

	m_Socket = std::make_unique<ix::WebSocket>();
	m_Socket->setUrl(std::format("wss://{}/chaos/ws?channel={}&token={}", server, channel, token));

	m_Socket->setOnMessageCallback(
	    [this](const ix::WebSocketMessagePtr &msg)
	    {
		    if (msg->type == ix::WebSocketMessageType::Message)
		    {
			    DEBUG_LOG("received message: " << msg->str);
			    HandleMessage(msg->str);
		    }
		    else if (msg->type == ix::WebSocketMessageType::Open)
		    {
			    LOG("Connection established");
		    }
		    else if (msg->type == ix::WebSocketMessageType::Error)
		    {
			    // Maybe SSL is not configured properly
			    LOG("Connection error: " << msg->errorInfo.reason);
			    if (!m_ErrorSplashShown)
			    {
				    ShowErrorSplash(
				        "Error when connecting to~n~channel point redemption server!~n~Check chaoslog.txt for details");
				    m_ErrorSplashShown = true;
			    }
		    }
	    });

	m_Socket->start();

	onEffectDispatchListener.Register(GetComponent<EffectDispatcher>()->OnPostDispatchEffect,
	                                  [this](const EffectIdentifier &effectId, const std::string context)
	                                  { OnEffectDispatched(effectId, context); });

	onEffectDispatchFailListener.Register(GetComponent<EffectDispatcher>()->OnDispatchEffectFailed,
	                                      [this](const EffectIdentifier &effectId, const std::string &context)
	                                      { OnEffectFailed(effectId, context); });

	for (const auto &effect : GetFilteredEnabledEffects())
		m_Effects.emplace_back(effect->Id, effect->HasCustomName() ? effect->CustomName : effect->Name, 0,
		                       !effect->IsExcludedFromCheatVoting());

	std::sort(m_Effects.begin(), m_Effects.end(),
	          [](const EffectEntry &a, const EffectEntry &b) { return StringCompare(a.Name, b.Name) > 0; });

	for (size_t i = 0; i < m_Effects.size(); i++)
		m_Effects[i].Index = i + 1;
}

void DispatchSocket::OnModPauseCleanup()
{
	if (m_Socket)
	{
		m_Socket->stop();
		m_Socket.release();
	}

	m_ErrorSplashShown = false;
}
