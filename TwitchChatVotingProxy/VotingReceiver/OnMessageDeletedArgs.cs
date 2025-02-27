namespace TwitchChatVotingProxy.VotingReceiver
{
    /// <summary>
    /// Event which should be dispatched when the voting receiver receives
    /// a message.
    /// </summary>
    class OnMessageDeletedArgs
    {
        public string? MessageId { get; set; } = null;
        public string? Message { get; set; } = null;
    }
}
