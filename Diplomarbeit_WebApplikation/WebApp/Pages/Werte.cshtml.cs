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


namespace WebApp.Pages
{
    public class WerteModel : PageModel
    {
        private GetArduinoData getArduinoData = new GetArduinoData();

        public string Temperature { get; set; }
        public string Humidity { get; set; }    

        public void OnGet()
        {
            this.Temperature = getArduinoData.Temperature;
            this.Humidity = getArduinoData.Humidity;
        }
    }
}
