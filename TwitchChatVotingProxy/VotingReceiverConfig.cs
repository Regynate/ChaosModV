using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VotingProxy
{
    /// <summary>
    /// Config object for the twitch implementation of the voting receiver
    /// </summary>
    class VotingReceiverConfig
    {
        public VotingReceiverConfig(string channelId, string oAuth, string userName, EChannelType channelType, string clientId, string userId)
        {
            // Validate config
            if (channelId == null) throw new Exception("channel name cannot be null");
            if (oAuth == null) throw new Exception("oAuth cannot be null");
            if (userName == null) throw new Exception("user name cannot be null");

            ChannelId = channelId;
            ClientId = clientId;
            OAuth = oAuth;
            UserName = userName;
            ChannelType = channelType;
            UserId = userId;
        }

        public string ChannelId { get; set; }
        public string ClientId { get; set; }
        public string UserId { get; set; }
        public string OAuth { get; set; }
        public string UserName { get; set; }
        public EChannelType ChannelType { get; set; }
    }
}
