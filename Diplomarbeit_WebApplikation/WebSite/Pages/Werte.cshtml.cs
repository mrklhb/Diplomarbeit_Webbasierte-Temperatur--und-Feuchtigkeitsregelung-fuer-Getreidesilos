using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;

namespace WebSite.Pages
{
    public class WerteModel : PageModel
    {
        private GetArduinoData getArduinoData = new GetArduinoData();

        public string Temperature { get; set; }
        public string Humidity { get; set; }

        public void OnGet()
        {
            Temperature = getArduinoData.Temperature;
            Humidity = getArduinoData.Humidity;
            System.Diagnostics.Debug.WriteLine(Temperature);
            System.Diagnostics.Debug.WriteLine(Humidity);
        }
    }
}
