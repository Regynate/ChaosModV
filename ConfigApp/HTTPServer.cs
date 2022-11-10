using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Linq;
using System.Runtime.Remoting.Contexts;
using System.IO;

namespace ConfigApp
{
    class HTTPServer
    {
        private static HttpListener listener;
        private static string url = "http://localhost:8876/";
        private bool running = false;

        private TwitchAuth m_twitchAuth;

        public HTTPServer(TwitchAuth auth)
        {
            listener = new HttpListener();
            listener.Prefixes.Add(url);
            listener.Start();
            running = true;

            m_twitchAuth = auth;

            _ = HandleIncomingConnections();
        }

        private async Task HandleIncomingConnections()
        {
            while (running)
            {
                HttpListenerContext ctx = await listener.GetContextAsync();

                HttpListenerRequest req = ctx.Request;
                HttpListenerResponse res = ctx.Response;

                byte[] data = new byte[0];

                if (req.Url.AbsolutePath == "/error")
                {
                    data = File.ReadAllBytes(".\\error.html");
                }
                else if (req.Url.AbsolutePath == "/oauth/redirect")
                {
                    data = File.ReadAllBytes(".\\redirect.html");
                }
                else if (req.Url.AbsolutePath == "/oauth")
                {
                    data = File.ReadAllBytes(".\\success.html");
                    string token = req.QueryString.Get("access_token");
                    m_twitchAuth.SetOauthToken(token);
                }

                await res.OutputStream.WriteAsync(data, 0, data.Length);
                res.Close();
            }
        }

        public void Stop()
        {
            running = false;
            listener.Stop();
        }
    }
}
