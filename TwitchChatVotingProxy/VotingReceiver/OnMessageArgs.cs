﻿namespace TwitchChatVotingProxy.VotingReceiver
{
    /// <summary>
    /// Event which should be dispatched when the voting receiver receives
    /// a message.
    /// </summary>
    class OnMessageArgs
    {
        public string? MessageId { get; set; } = null;
        public string? Message { get; set; } = null;
        public string? UserId { get; set; } = null;
        public string? Username { get; set; } = null;
        public string? DisplayName { get; set; } = null;
        public string? Color { get; set; } = null;
    }
}
