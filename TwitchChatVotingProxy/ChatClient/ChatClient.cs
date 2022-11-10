using System;
using System.Threading.Tasks;

namespace VotingProxy.VotingChatClient
{
    public abstract class ChatClient
    {
        public virtual event EventHandler OnConnected;
        public virtual event EventHandler OnDisconnect;
        public virtual event EventHandler<OnErrorEventArgs> OnError;
        public virtual event EventHandler OnIncorrectLogin;
        public virtual event EventHandler OnJoinedChannel;
        public virtual event EventHandler<OnMessageReceivedArgs> OnMessageReceived;

        public abstract void SendMessage(string channelId, string message);
        public abstract Task<string> StartPoll(string channelId, string name, string[] options);
        public abstract Task<int[]> EndPoll(string channelId, string pollId);
        public abstract void Connect();
        public abstract void Disconnect();
        public abstract void Initialize(Credentials credentials, string ChannelId);
    }

    public class Credentials
    {
        public readonly string UserName;
        public readonly string OAuth;
        public readonly string ClientId;
        public readonly string UserId;
        public Credentials(string userName, string oAuth, string clientId, string userId)
        {
            OAuth = oAuth;
            UserName = userName;
            ClientId = clientId;
            UserId = userId;
        }
    }

    public class OnErrorEventArgs : EventArgs
    {
        public Exception Exception;
    }

    public class OnMessageReceivedArgs : EventArgs
    {
        public ChatMessage ChatMessage;
    }

    public class ChatMessage
    {
        public string Message { get; set; }
        public string UserId { get; set; }
        public string Username { get; set; }
    }
}
