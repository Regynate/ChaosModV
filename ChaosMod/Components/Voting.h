#pragma once

#include "Components/Component.h"
#include "Util/Chat/ChatMessage.h"
#include "Util/Chat/ChatMessageDeletion.h"
#include "Util/Chat/ChatUserBan.h"
#include "Util/VotingMode.h"

#include "Util/Events.h"

#include <cstdint>
#include <memory>
#include <vector>

class Voting : public Component
{
	std::string m_VoteablePrefix;

	struct ChoosableEffect
	{
		EffectIdentifier Id;
		std::string Name;
		std::string Match;
		int ChanceVotes = 0;

		ChoosableEffect(const EffectIdentifier &effectId, const std::string &name, const std::string &match)
		    : Id(effectId), Name(name), Match(match)
		{
		}
	};
	std::vector<std::unique_ptr<ChoosableEffect>> m_EffectChoices;

	std::uint64_t m_LastPing           = GetTickCount64();
	std::uint64_t m_LastVotesFetchTime = GetTickCount64();

	void *m_PipeHandle                 = INVALID_HANDLE_VALUE;

	std::unique_ptr<EffectIdentifier> m_ChosenEffectId;

	int m_SecsBeforeVoting = 0;

	int m_NoPingRuns       = 0;

	enum class OverlayMode
	{
		ChatMessages,
		OverlayIngame,
		OverlayOBS
	} m_OverlayMode = OverlayMode::ChatMessages;

	std::array<std::uint8_t, 3> m_TextColor;

	bool m_EnableVoting                                = false;
	bool m_HasInitializedVoting                        = false;

	bool m_ReceivedHello                               = false;
	bool m_HasReceivedResult                           = false;

	bool m_IsVotingRoundDone                           = true;
	bool m_AlternatedVotingRound                       = false;

	VotingMode m_VotingMode                            = VotingMode::Majority;
	bool m_EnableVotingChanceSystemRetainInitialChance = true;
	bool m_EnableRandomEffectVoteable                  = true;

	bool m_IsVotingRunning                             = false;

	VotingMode m_VotingModeOverride                    = VotingMode::None;

  public:
	Voting(const std::array<std::uint8_t, 3> &TextColor);

	bool Init();
	bool IsEnabled() const;
	VotingMode GetVotingMode() const;
	void HandleMsg(std::string_view message);

	ChaosEvent<const ChatMessage &> OnNewMessage;
	ChaosEvent<const ChatMessageDeletion &> OnMessageDelete;
	ChaosEvent<const ChatUserBan &> OnUserBan;

  private:
	std::string GetPipeJson(std::string_view identifier, std::vector<std::string> params);

  public:
	void SendToPipe(std::string_view identifier, std::vector<std::string> params = {});
	void ErrorOutWithMsg(std::string_view message);

	virtual void OnModPauseCleanup() override;
	virtual void OnRun() override;
};