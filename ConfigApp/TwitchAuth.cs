using System;
using System.IO;
using System.Windows;
using System.Net.Http;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Net.Http.Headers;
using Newtonsoft.Json;

namespace ConfigApp
{
    class TwitchAuth
    {
        private MainWindow m_Main;
        private HTTPServer m_httpServer = null;
        private HttpClient m_httpClient;

        private string m_twitchOauth = "";
        private string m_twitchUsername = "";

        private static string m_twitchLoginUrl = "https://id.twitch.tv/oauth2/authorize?client_id={0}&redirect_uri={1}&force_verify=true&response_type=token&scope=chat:read+channel:manage:polls+channel:read:polls";
        private static string m_twitchRedirectUri = "http://localhost:8876/oauth/redirect";
        private string m_twitchAppClientId = "";
        private string m_twitchClientSecret = "";
        private string m_formattedTwitchLoginUrl = "";
        private string m_twitchClientId = "";
        private string m_twitchUserId = "";

        public TwitchAuth(MainWindow main)
        {
            m_Main = main;
            m_httpClient = new HttpClient();

            if (m_httpServer == null)
            {
                m_httpServer = new HTTPServer(this);
            }

            // DotEnv Handling

            string root = Directory.GetCurrentDirectory();
            string dotenv = Path.Combine(root, ".env");

            if (File.Exists(dotenv))
            {
                DotEnv.Load(dotenv);

                m_twitchAppClientId = Environment.GetEnvironmentVariable("TWITCH_CLIENT_ID");
                m_twitchClientSecret = Environment.GetEnvironmentVariable("TWITCH_CLIENT_SECRET");
                m_formattedTwitchLoginUrl = string.Format(m_twitchLoginUrl, m_twitchAppClientId, m_twitchRedirectUri);
            }
        }

        private bool m_loggedIn()
        {
            return m_twitchOauth != null && m_twitchOauth.Length > 0;
        }

        public string OAuthToken
        {
            get => m_twitchOauth;
            set => m_twitchOauth = value;
        }

        public string ClientID
        {
            get => m_twitchClientId;
            set => m_twitchClientId = value;
        }

        public string Username
        {
            get => m_twitchUsername;
            set => m_twitchUsername = value;
        }

        public string UserId
        {
            get => m_twitchUserId;
            set => m_twitchUserId = value;
        }

        public bool LoggedIn
        {
            get => m_loggedIn();
        }

        public async Task<bool> ValidateToken()
        {
            if (LoggedIn)
            {
                var requestMessage = new HttpRequestMessage(HttpMethod.Get, "https://id.twitch.tv/oauth2/validate");
                HttpResponseMessage response = await m_httpClient.SendAsync(requestMessage);
                if (response.IsSuccessStatusCode)
                {
                    string text = await response.Content.ReadAsStringAsync();
                    var dict = JsonConvert.DeserializeObject<Dictionary<string, object>>(text);
                    m_twitchClientId = (string)dict["client_id"];
                    m_twitchUsername = (string)dict["login"];
                    m_twitchUserId = (string)dict["user_id"];
                    return true;
                }
            }
            return false;
        }

        public void SpawnLogin()
        {
#if DEBUG
            if (m_twitchAppClientId.Length == 0)
            {
                MessageBox.Show("No TWITCH_CLIENT_ID present. You may be missing your .env file. Skipping login.", "Cannot log in with Twitch", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            else if (m_twitchClientSecret.Length == 0)
            {
                MessageBox.Show("No TWITCH_CLIENT_SECRET present. You may be missing your .env file. Skipping login.", "Cannot log in with Twitch", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
#endif

            System.Diagnostics.Process.Start(m_formattedTwitchLoginUrl);
        }

        public async void SetOauthToken(string token)
        {
            m_twitchOauth = token;
            m_httpClient.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("OAuth", token);

            if (!await ValidateToken())
            {
                throw new Exception("Invalid token");
            }

            m_Main.WriteTwitchFile();
            m_Main.SetupTwitchLogin();
        }

        public void StopServer()
        {
            if (m_httpServer != null)
            {
                m_httpServer.Stop();
            }
        }
    }
}
