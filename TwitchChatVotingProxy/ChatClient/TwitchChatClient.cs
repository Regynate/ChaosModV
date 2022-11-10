using Serilog.Core;
using System;
using System.CodeDom.Compiler;
using System.Linq;
using TwitchLib.Api;
using TwitchLib.Api.Helix.Models.Polls.CreatePoll;
using TwitchLib.Client;
using TwitchLib.Client.Events;
using TwitchLib.Client.Models;
using TwitchLib.Communication.Clients;
using System.Threading.Tasks;
using System.IO;

namespace VotingProxy.VotingChatClient
{

    class TwitchChatClient : ChatClient
    {
        private TwitchClient client;
        private TwitchAPI api;

        public TwitchChatClient()
        {
            client = new TwitchClient(new WebSocketClient());
            api = new TwitchAPI();
        }

        public override void Initialize(Credentials credentials, string ChannelId)
        {
            client.Initialize(
                new ConnectionCredentials(credentials.UserName, credentials.OAuth),
                ChannelId);
            api.Settings.AccessToken = credentials.OAuth;
            api.Settings.ClientId = credentials.ClientId;
        }

        public override void Connect()
        {
            client.Connect();
        }

        public override void Disconnect()
        {
            client.Disconnect();
        }

        public override void SendMessage(string channelId, string message)
        {
            client.SendMessage(channelId, message);
        }

        public override async Task<string> StartPoll(string userId, string name, string[] options)
        {
            var poll = await api.Helix.Polls.CreatePollAsync(
                new CreatePollRequest()
                {
                    BroadcasterId = userId,
                    Title = name,
                    Choices = Array.ConvertAll(options, o => new Choice() { Title = o }),
                    DurationSeconds = 1800
                });
            return poll.Data[0].Id;
        }

        public override async Task<int[]> EndPoll(string userId, string pollId)
        {
            var polls = await api.Helix.Polls.GetPollsAsync(userId, new string[] { pollId }.ToList());
            if (polls.Data.Length == 0)
            {
                throw new Exception("no poll");
            }
            else if (polls.Data.Length > 1)
            {
                throw new Exception("too many polls");
            }
            var choices = polls.Data[0].Choices;
            await api.Helix.Polls.EndPollAsync(userId, pollId, TwitchLib.Api.Core.Enums.PollStatusEnum.TERMINATED);
            return Array.ConvertAll(choices, c => c.Votes);
        }

        public sealed override event EventHandler OnConnected
        {
            add
            {
                client.OnConnected += new EventHandler<OnConnectedArgs>(value);
            }
            remove
            {
                client.OnConnected -= new EventHandler<OnConnectedArgs>(value);
            }
        }

        public sealed override event EventHandler<OnErrorEventArgs> OnError
        {
            add
            {
                client.OnError += (object sender, TwitchLib.Communication.Events.OnErrorEventArgs e) =>
                {
                    OnErrorEventArgs evt = new OnErrorEventArgs();
                    evt.Exception = e.Exception;

                    value(sender, evt);
                };
            }
            remove
            {
                client.OnError -= (object sender, TwitchLib.Communication.Events.OnErrorEventArgs e) =>
                {
                    OnErrorEventArgs evt = new OnErrorEventArgs();
                    evt.Exception = e.Exception;

                    value(sender, evt);
                };
            }
        }

        public sealed override event EventHandler<OnMessageReceivedArgs> OnMessageReceived
        {
            add
            {
                client.OnMessageReceived += (object sender, TwitchLib.Client.Events.OnMessageReceivedArgs e) =>
                {
                    ChatMessage e_ChatMessage = new ChatMessage()
                    {
                        Message = e.ChatMessage.Message,
                        UserId = e.ChatMessage.UserId,
                        Username = e.ChatMessage.Username
                    };
                    OnMessageReceivedArgs evt = new OnMessageReceivedArgs()
                    {

                        ChatMessage = e_ChatMessage
                    };

                    value(sender, evt);
                };
            }
            remove
            {
                client.OnMessageReceived -= (object sender, TwitchLib.Client.Events.OnMessageReceivedArgs e) =>
                {
                    ChatMessage e_ChatMessage = new ChatMessage()
                    {
                        Message = e.ChatMessage.Message,
                        UserId = e.ChatMessage.UserId,
                        Username = e.ChatMessage.Username
                    };
                    OnMessageReceivedArgs evt = new OnMessageReceivedArgs()
                    {

                        ChatMessage = e_ChatMessage
                    };

                    value(sender, evt);
                };
            }
        }

        public sealed override event EventHandler OnIncorrectLogin
        {
            add
            {
                client.OnIncorrectLogin += new EventHandler<OnIncorrectLoginArgs>(value);
            }
            remove
            {
                client.OnIncorrectLogin -= new EventHandler<OnIncorrectLoginArgs>(value);
            }
        }

        public sealed override event EventHandler OnJoinedChannel
        {
            add
            {
                client.OnJoinedChannel += new EventHandler<OnJoinedChannelArgs>(value);
            }
            remove
            {
                client.OnJoinedChannel -= new EventHandler<OnJoinedChannelArgs>(value);
            }
        }
    }
}
