using System.Net.Sockets;
using System.Text;
using System.Net;
using System.Threading.Tasks;

namespace WebSite
{
    public class GetArduinoData
    {
        public string Temperature { get; set; }
        public string Humidity { get; set; }

        private int port = 9999;
        private IPEndPoint receiveAdr;    // IP: 0.0.0.0
        private UdpClient udpClient;
        
        // neue Empfangstheorie
        private bool _isReceiving;
        private UdpClient _udpClient;
        private CancellationTokenSource _cancellationTokenSource;

        public GetArduinoData()
        {
            //receiveAdr = new IPEndPoint(IPAddress.Any, port);
            //udpClient = new UdpClient(receiveAdr);
            // ReceiveAsync();
            _ = StartReceivingData();
        }
        //private async void ReceiveAsync()
        //{
        //    while (true)
        //    {
        //        UdpReceiveResult result = await udpClient.ReceiveAsync();
        //        string msg = Encoding.UTF8.GetString(result.Buffer);
        //        string[] splitted = msg.Split("!");
        //        if (splitted.Length < 6)
        //        {
        //            continue;
        //        }
        //        Temperature = splitted[0];
        //        Humidity = splitted[1];
        //    }
        //}

        private async Task StartReceivingData()
        {
            _isReceiving = true;
            _udpClient = new UdpClient(port);
            _cancellationTokenSource = new CancellationTokenSource();

            await Task.Run(async () =>
            {
                while (_isReceiving)
                {
                    try
                    {
                        var receivedData = await _udpClient.ReceiveAsync();
                        string msg = Encoding.UTF8.GetString(receivedData.Buffer);
                        string[] splitted = msg.Split("!");
                        Temperature = splitted[0];
                        Humidity = splitted[1];

                    }
                    catch (SocketException se)
                    {
                        Console.WriteLine($"SocketException: {se}");
                    }
                    catch (OperationCanceledException)
                    {
                        Console.WriteLine("Empfangen abgebrochen");
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error: {ex}");
                    }
                }
            });
        }
    }
}

