using System.Windows;
using System.Windows.Controls;

namespace ConfigApp.Tabs.Voting
{
    public class TwitchTab : Tab
    {
        private CheckBox? m_EnableTwitchVoting = null;
        private CheckBox? m_EnableChannelPoints = null;
        private TextBox? m_ChannelName = null;
        private TextBox? m_ChannelPointsServer = null;
        private PasswordBox? m_ChannelPointsToken = null;

        private void SetElementsEnabled(bool state)
        {
            if (m_ChannelName is not null)
                m_ChannelName.IsEnabled = state;
            if (m_EnableChannelPoints is not null)
                m_EnableChannelPoints.IsEnabled = state;
            if (m_ChannelPointsServer is not null)
                m_ChannelPointsServer.IsEnabled = state;
            if (m_ChannelPointsToken is not null)
                m_ChannelPointsToken.IsEnabled = state;
        }

        protected override void InitContent()
        {
            PushNewColumn(new GridLength(340f));
            PushNewColumn(new GridLength(10f));
            PushNewColumn(new GridLength(150f));
            PushNewColumn(new GridLength(250f));
            PushNewColumn(new GridLength(10f));
            PushNewColumn(new GridLength());

            PushRowEmpty();
            PushRowEmpty();
            PushRowEmpty();
            m_EnableTwitchVoting = new CheckBox()
            {
                HorizontalAlignment = HorizontalAlignment.Left,
                VerticalAlignment = VerticalAlignment.Center,
                Content = "Enable Twitch Voting"
            };
            m_EnableTwitchVoting.Click += (sender, eventArgs) =>
            {
                SetElementsEnabled(m_EnableTwitchVoting.IsChecked.GetValueOrDefault());
            };
            PushRowElement(m_EnableTwitchVoting);
            PopRow();

            PushRowSpacedPair("Channel Name", m_ChannelName = new TextBox()
            {
                Width = 120f,
                Height = 20f
            });
            PopRow();

            m_EnableChannelPoints = new CheckBox()
            {
                HorizontalAlignment = HorizontalAlignment.Left,
                VerticalAlignment = VerticalAlignment.Center
            };
            PushRowSpacedPair("Enable channel point redemption", m_EnableChannelPoints);
            PopRow();

            PushRowSpacedPair("Channel Points Server", m_ChannelPointsServer = new TextBox()
            {
                Width = 120f,
                Height = 20f
            });

            PushRowSpacedPair("Channel Points Token", m_ChannelPointsToken = new PasswordBox()
            {
                Width = 120f,
                Height = 20f
            });
            PopRow();

            SetElementsEnabled(false);
        }

        public override void OnLoadValues()
        {
            if (m_EnableTwitchVoting is not null)
            {
                m_EnableTwitchVoting.IsChecked = OptionsManager.VotingFile.ReadValue("EnableVotingTwitch", false);
                SetElementsEnabled(m_EnableTwitchVoting.IsChecked.GetValueOrDefault());
            }
            if (m_EnableChannelPoints is not null)
                m_EnableChannelPoints.IsChecked = OptionsManager.VotingFile.ReadValue("EnableChannelPoints", false);
            if (m_ChannelName is not null)
                m_ChannelName.Text = OptionsManager.VotingFile.ReadValue<string>("TwitchChannelName");
            if (m_ChannelPointsServer is not null)
                m_ChannelPointsServer.Text = OptionsManager.VotingFile.ReadValue<string>("ChannelPointsServer", "regynate.com");
            if (m_ChannelPointsToken is not null)
                m_ChannelPointsToken.Password = OptionsManager.VotingFile.ReadValue<string>("ChannelPointsToken");
        }

        public override void OnSaveValues()
        {
            OptionsManager.VotingFile.WriteValue("EnableVotingTwitch", m_EnableTwitchVoting?.IsChecked);
            OptionsManager.VotingFile.WriteValue("EnableChannelPoints", m_EnableChannelPoints?.IsChecked);
            OptionsManager.VotingFile.WriteValue("TwitchChannelName", m_ChannelName?.Text);
            OptionsManager.VotingFile.WriteValue("ChannelPointsServer", m_ChannelPointsServer?.Text);
            OptionsManager.VotingFile.WriteValue("ChannelPointsToken", m_ChannelPointsToken?.Password);
        }
    }
}
