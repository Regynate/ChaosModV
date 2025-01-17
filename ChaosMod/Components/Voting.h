#pragma once

#include "Components/Component.h"

#include <cstdint>
#include <memory>
#include <vector>

class Voting : public Component
{
	std::string m_VoteablePrefix;

	struct ChoosableEffect
	{
		ChoosableEffect(const EffectIdentifier &effectIdentifier, const std::string &name, const std::string &match)
		    : m_EffectIdentifier(effectIdentifier), m_EffectName(name), m_Match(match)
		{
		}

		EffectIdentifier m_EffectIdentifier;
		std::string m_EffectName;
		std::string m_Match;
		int m_ChanceVotes = 0;
	};
	std::vector<std::unique_ptr<ChoosableEffect>> m_EffectChoices;

	std::uint64_t m_LastPing           = GetTickCount64();
	std::uint64_t m_LastVotesFetchTime = GetTickCount64();

	void *m_PipeHandle                 = INVALID_HANDLE_VALUE;

	std::unique_ptr<EffectIdentifier> m_ChosenEffectIdentifier;

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

	bool m_EnableChanceSystem                          = false;
	bool m_EnableVotingChanceSystemRetainInitialChance = true;
	bool m_EnableRandomEffectVoteable                  = true;

	bool m_IsVotingRunning                             = false;

  public:
	Voting(const std::array<std::uint8_t, 3> &TextColor);

	bool Init();
	bool IsEnabled() const;
	void HandleMsg(std::string_view message);

  private:
	std::string GetPipeJson(std::string_view identifier, std::vector<std::string> params);

  public:
	void SendToPipe(std::string_view identifier, std::vector<std::string> params = {});
	void ErrorOutWithMsg(std::string_view message);

	virtual void OnModPauseCleanup() override;
	virtual void OnRun() override;
};