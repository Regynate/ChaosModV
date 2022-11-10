using System;
using System.Threading.Tasks;

namespace VotingProxy.VotingReceiver
{
    /// <summary>
    /// Defines the interface that a voting receiver needs to satisfy
    /// </summary>
    interface IVotingReceiver
    {
        /// <summary>
        /// Events which get invoked when the voting receiver receives a message
        /// </summary>
        event EventHandler<OnMessageArgs> OnMessage;

        /// <summary>
        /// Sends a message to the connected service
        /// </summary>
        /// <param name="message">Message that should be sent</param>
        void SendMessage(string message);

        /// <summary>
        /// Starts a poll on the device
        /// </summary>
        /// <param name="name">Header of the poll</param>
        /// <param name="options">Poll options</param>
        Task<string> StartPoll(string name, string[] options);

        Task<int[]> EndPoll(string pollId);

        bool JoinedChannel();
    }
}
