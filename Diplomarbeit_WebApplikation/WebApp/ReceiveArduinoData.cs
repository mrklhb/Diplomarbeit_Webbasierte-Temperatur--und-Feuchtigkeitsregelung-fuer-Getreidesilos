using System.Net.Sockets;
using System.Text;
using System.Net;

namespace WebApp
{
    public class GetArduinoData
    {
        public string Temperature { get; set; }
        public string Humidity { get; set; }

        private int port = 9999;
        private IPEndPoint receiveAdr;    // IP: 0.0.0.0
        private UdpClient udpClient;

        public GetArduinoData()
        {
            receiveAdr = new IPEndPoint(IPAddress.Any, port);
            udpClient = new UdpClient(receiveAdr);
            ReceiveAsync();
        }
        private async void ReceiveAsync()
        {
            while (true)
            {
                UdpReceiveResult result = await udpClient.ReceiveAsync();
                string msg = Encoding.UTF8.GetString(result.Buffer);
                string[] splitted = msg.Split("!");
                if (splitted.Length < 6)
                {
                    continue;
                }
                Temperature = splitted[0];
                Humidity = splitted[1];
            }
        }
    }
}
