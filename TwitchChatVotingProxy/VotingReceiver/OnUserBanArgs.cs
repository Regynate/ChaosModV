namespace TwitchChatVotingProxy.VotingReceiver
{
    /// <summary>
    /// Event which should be dispatched when the voting receiver receives
    /// a message.
    /// </summary>
    class OnUserBanArgs
    {
        public string? Username { get; set; } = null;
        public string? UserId { get; set; } = null;
    }
}
