using System.Net.Sockets;
using System.Text;

namespace WebApp
{
    public class GetArduinoData
    {
        private int port = 9999;
        private UdpClient udpClient;
        public string msg;
        public string[] splitted;
        public decimal Temperature { get; set; }
        public decimal Humidity { get; set; }

        ReceiveArduinoData()
        {
            ReceiveAsync();
        }
        public async void ReceiveAsync()
        {
            while (true)
            {
                UdpReceiveResult result = await udpClient.ReceiveAsync();
                msg = Encoding.UTF8.GetString(result.Buffer);
                splitted = msg.Split("!");
                if (splitted.Length < 6)
                {
                    continue;
                }
                Temperature = decimal.Parse(splitted[0]);
                Humidity = decimal.Parse(splitted[1]);
            }
        }
    }
}
