using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics.Eventing.Reader;
using System.Drawing;
using System.Globalization;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace DataRead
{
    class Read
    {
        private int port = 9999;
        private UdpClient udpClient;
        public string msg;
        public string[] splitted;
        public decimal Temperature {get; set; }
        public decimal Humidity {get; set; }

        private async void ReceiveAsync()
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

namespace WebApp.Pages
{
    
    public class WerteModel : PageModel
    {
        public void OnGet(decimal temperature, decimal humidity)
        {
            Temperatur = temperature;
            Humidity = humidity;
        }
    }
}
