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

        private async Task StartReceivingData() // wird regelmäßig aufgerufen
        {
            Temperature = "nA.";
            Humidity = "nA.";
            _isReceiving = true;
            //
            _cancellationTokenSource = new CancellationTokenSource();
            int c = 0;
            while (_isReceiving) // braucht ihr die Schleife? wird sowieso regelmäßig aufgerufen!
            {
                await Task.Run(async () =>
                {
                    c++;
                    Temperature = c.ToString();
                    try
                    {
                        _udpClient = new UdpClient(port); // wenn hier nichts kommt, dann ist die Message leer --> Temperatur und Humidity werden geleert, aber oben wieder mit "nA." gefuellt.
                        if (_udpClient.Available > 0)
                        {
                            var receivedData = await _udpClient.ReceiveAsync();
                            string msg = Encoding.UTF8.GetString(receivedData.Buffer);

                            if (msg.Length > 0)
                            {
                                string[] splitted = msg.Split("!");
                                Temperature = splitted[0];
                                Humidity = splitted[1];
                            }
                        }
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

                });
            }
        }
    }
}

