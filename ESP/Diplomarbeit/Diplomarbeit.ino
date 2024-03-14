#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <WiFi.h>
#include "ESPAsyncWebServer.h"



#define fan_relay_gpio 25
#define PWM_CH_FAN  2
#define PWM_RES     8
#define MAX_DATA_POINTS 30

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define SPI_CS 5
#define SPI_CS1 15

// Replace with your network credentials
const char* ssid = "WiFi-Stones";
const char* password = "stones4net";
String modi = "";

String fan_state;
Adafruit_BME680 bme; // I2C

Adafruit_BME680 bmeSPI(SPI_CS, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME680 bmeSPI2(SPI_CS1, SPI_MOSI, SPI_MISO, SPI_SCK);
float temperature;
float humidity;
float pressure;
float gasResistance;
float temperatureSPI;
float humiditySPI;
float temperatureSPI2;
float humiditySPI2;
float temperatureSum = 0, humiditySum = 0;
float temperatureSumSPI = 0, humiditySumSPI = 0;
float temperatureSumSPI2 = 0, humiditySumSPI2 = 0;
int dataPointsCount = 0;
int dataPointsCountSPI = 0;
int dataPointsCountSPI2 = 0;
AsyncWebServer server(80);
AsyncEventSource events("/events");

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;  // send readings timer

void getBME680Readings() {
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  if (!bmeSPI.performReading()) {
    Serial.println(F("Failed to perform reading from BME680 SPI :("));
    return;
  }
  if (!bmeSPI2.performReading()) {
    Serial.println(F("Failed to perform reading from BME680 SPI2 :("));
    return;
  }
  temperature = bme.temperature;
  pressure = bme.pressure / 100.0;
  humidity = bme.humidity;
  gasResistance = bme.gas_resistance / 1000.0;

  temperatureSPI = bmeSPI.temperature;
  humiditySPI = bmeSPI.humidity;
  temperatureSPI2 = bmeSPI2.temperature;
  humiditySPI2 = bmeSPI2.humidity;


  if (dataPointsCount < MAX_DATA_POINTS) {
    temperatureSum += temperature;
    humiditySum += humidity;
    dataPointsCount++;
  }

  if (dataPointsCountSPI < MAX_DATA_POINTS) {
    temperatureSumSPI += temperatureSPI;
    humiditySumSPI += humiditySPI;
    dataPointsCountSPI++;
  }
  if (dataPointsCountSPI2 < MAX_DATA_POINTS) {
    temperatureSumSPI2 += temperatureSPI2;
    humiditySumSPI2 += humiditySPI2;
    dataPointsCountSPI2++;
  }
}

void calculateAverages() {
  if (dataPointsCount > 0 && dataPointsCountSPI > 0) {
    float avgTemperature = (temperatureSum + temperatureSumSPI + temperatureSumSPI2) / (dataPointsCount + dataPointsCountSPI + dataPointsCountSPI2);
    float avgHumidity = (humiditySum + humiditySumSPI + humiditySumSPI2) / (dataPointsCount + dataPointsCountSPI + dataPointsCountSPI2);

    events.send(String(avgTemperature).c_str(), "avgTemperature", millis());
    events.send(String(avgHumidity).c_str(), "avgHumidity", millis());
    temperatureSum = humiditySum = temperatureSumSPI = humiditySumSPI = temperatureSumSPI2 = humiditySumSPI2 = 0;
    dataPointsCount = dataPointsCountSPI = dataPointsCountSPI2 = 0;

  }
}

String processor(const String& var) {
  if (var == "TEMPERATURE") {
    return String(temperature);
  }
  else if (var == "HUMIDITY") {
    return String(humidity);
  }
  else if (var == "PRESSURE") {
    return String(pressure);
  }
  else if (var == "GAS") {
    return String(gasResistance);
  }
  if (var == "fan_state_web") {
    if (digitalRead(fan_relay_gpio)) {
      fan_state = "On";
    }
    else {
      fan_state = "Off";
    }
    return fan_state;
  }
  return String();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Diplomarbeit Feuchtigkeit und Temperatur im Getreidehochsilo</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {
  font-family: Arial; 
  display: inline-block; 
  text-align: center;
}

p {
  font-size: 1.2rem;
}

body {
  margin: 0;
}

.topnav {
  overflow: hidden; 
  background-color: #4B1D3F; 
  color: white; 
  font-size: 1.7rem; 
}

.content {
    padding: 20px;
    text-align: center; 
  }

.cards {
  max-width: 700px; 
  margin: 0 auto; 
  display: grid;
  grid-template-columns: repeat(2, 1fr); 
  grid-gap: 1rem; 
}

.card {
  background-color: white; 
  box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
  border: 5px solid; 
  margin: 10px;
  padding: 10px;
}


.card.temperature.top, .card.humidity.top { border-color: #FF0000; } /* Rot */
.card.temperature.middle, .card.humidity.middle { border-color: #FFD700; } /* Gelb */
.card.temperature.bottom, .card.humidity.bottom { border-color: #008000; } /* Grün */
.card.averageTemperature, .card.averageHumidity { border-color: #000000; } /* Schwarz */

.reading {
  font-size: 2rem;
}

.button {
  display: inline-block; 
  background-color: #008CBA; 
  border: none; 
  border-radius: 4px; 
  color: white; 
  padding: 6px 20px; 
  text-decoration: none; 
  font-size: 15px; 
  margin: 2px; 
  cursor: pointer;
}

.button:hover {
  background-color: #005f73;
  color: #d1e8e2;
}

.button2 {
  background-color: #f44336;
}

 canvas {
    min-width: 500px;
    min-height: 300px;
    display: block; 
    margin: 20px auto; 
  }


@media (max-width: 600px) {
  .cards {
    grid-template-columns: 1fr; 
  }
}
  </style>
</head>
<body>
  <div class="topnav">
    <h3>Diplomarbeit Feuchtigkeit und Temperatur im Getreidehochsilo</h3>
  </div>
  <div class="content">
    <div class="cards">
  <!-- Temperatur und Feuchtigkeit OBEN in Rot -->
  <div class="card temperature top">
    <h4><i class="fas fa-thermometer-half"></i> Temperatur OBEN</h4><p><span class="reading"><span id="temp">%TEMPERATURE%</span> &deg;C</span></p>
  </div>
  <div class="card humidity top">
    <h4><i class="fas fa-tint"></i> Feuchtigkeit OBEN</h4><p><span class="reading"><span id="hum">%HUMIDITY%</span> &percnt;</span><br></p>
  </div>

  <!-- Temperatur und Feuchtigkeit MITTE in Gelb -->
  <div class="card temperature middle">
    <h4><i class="fas fa-thermometer-half"></i> Temperatur MITTE</h4><p><span class="reading"><span id="tempSPI2">%TEMPERATURESPI2%</span> &deg;C</span></p>
  </div>
  <div class="card humidity middle">
    <h4><i class="fas fa-tint"></i> Feuchtigkeit MITTE</h4><p><span class="reading"><span id="humSPI2">%HUMIDITYSPI2%</span> &percnt;</span><br></p>
  </div>

  <!-- Temperatur und Feuchtigkeit UNTEN in Grün -->
  <div class="card temperature bottom">
    <h4><i class="fas fa-thermometer-half"></i> Temperatur UNTEN</h4><p><span class="reading"><span id="tempSPI">%TEMPERATURESPI%</span> &deg;C</span></p>
  </div>
  <div class="card humidity bottom">
    <h4><i class="fas fa-tint"></i> Feuchtigkeit UNTEN</h4><p><span class="reading"><span id="humSPI">%HUMIDITYSPI%</span> &percnt;</span></p>
  </div>

        <div class="card averageTemperature">
        <h4>Temperatur Durchschnitt</h4><p><span id="avgTemp">%AVGTEMPERATURE%</span> &deg;C</p>
      </div>
      <div class="card averageHumidity">
        <h4>Feuchtigkeit Durchschnitt</h4><p><span id="avgHum">%AVGHUMIDITY%</span> &percnt;</p>
      </div>
  </div>
  <div>
      <h4>L&uumlftung:</h4>
      <p><a href="/fan_on"> <button class="button">On</button></a>
      <a href="/fan_off"> <button class="button button2">Off</button></a></p>
      <!-- Neuer Button für das Zurücksetzen der Daten -->
      <h4>Daten:</h4>
      <p><button class="button" onclick="confirmAction(event, 'resetData')">Daten von 1-Stunde Graphen zur&uumlsetzen</button></p>
      <p><button class="button" onclick="confirmAction(event, 'resetChartData')">Daten von 1-Tages Graphen zur&uumlsetzen</button></p>

      
      
    </div>
  <canvas id="temperatureChart" width="300" height="150"></canvas>
  <canvas id="humidityChart" width="300" height="150"></canvas>
  <canvas id="avgOfAvgsChart" width="300" height="150"></canvas>
  <canvas id="avgOfAvgsHumidityChart" width="300" height="150"></canvas>

  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script>
    if (!!window.EventSource) {
     var source = new EventSource('/events');

     source.addEventListener('open', function(e) {
      console.log("Events Connected");
     }, false);
     source.addEventListener('error', function(e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      }
     }, false);

     source.addEventListener('message', function(e) {
      console.log("message", e.data);
     }, false);

      source.addEventListener('temperature', function(e) {
        console.log("temperature", e.data);
        document.getElementById("temp").innerHTML = e.data;
      }, false);
     source.addEventListener('humidity', function(e) {
      console.log("humidity", e.data);
      document.getElementById("hum").innerHTML = e.data;
     }, false);
    source.addEventListener('temperatureSPI', function(e) {
    console.log("temperatureSPI", e.data);
    document.getElementById("tempSPI").innerHTML = e.data;
  }, false);


  source.addEventListener('humiditySPI', function(e) {
    console.log("humiditySPI", e.data);
    document.getElementById("humSPI").innerHTML = e.data;
  }, false);

 source.addEventListener('avgTemperature', function(e) {
        console.log("avgTemperature", e.data);
        document.getElementById("avgTemp").innerHTML = e.data;
        updateTemperatureChart(parseFloat(e.data));
      }, false);

      source.addEventListener('avgHumidity', function(e) {
        console.log("avgHumidity", e.data);
        document.getElementById("avgHum").innerHTML = e.data;
        updateHumidityChart(parseFloat(e.data));
      }, false);

      source.addEventListener('temperatureSPI2', function(e) {
  console.log("temperatureSPI2", e.data);
  document.getElementById("tempSPI2").innerHTML = e.data;
}, false);

source.addEventListener('humiditySPI2', function(e) {
  console.log("humiditySPI2", e.data);
  document.getElementById("humSPI2").innerHTML = e.data;
}, false);
  

      var ctxTemperature = document.getElementById('temperatureChart').getContext('2d');
      var chartTemperature = createChart(ctxTemperature, 'Temperatur 1 Stunde', 'rgb(255, 99, 132)');

      var ctxHumidity = document.getElementById('humidityChart').getContext('2d');
      var chartHumidity = createChart(ctxHumidity, 'Feuchtigkeit 1 Stunde', 'rgb(75, 192, 192)');

      var ctxAvgOfAvgs = document.getElementById('avgOfAvgsChart').getContext('2d');
      var chartAvgOfAvgs = createChart(ctxAvgOfAvgs, 'Temperatur 1 Tag', 'rgb(60, 179, 113)');

      var ctxAvgOfAvgsHumidity = document.getElementById('avgOfAvgsHumidityChart').getContext('2d');
      var chartAvgOfAvgsHumidity = createChart(ctxAvgOfAvgsHumidity, 'Feuchtigkeit 1 Tag', 'rgb(100, 149, 237)');



     // Setzt der Canvas-Größe nach dem Laden der Seite
     window.addEventListener('load', function() {
       document.getElementById('temperatureChart').width = 300;
       document.getElementById('temperatureChart').height = 150;
       document.getElementById('humidityChart').width = 300;
       document.getElementById('humidityChart').height = 150;
       

       // Ladet zuvor gespeicherte Daten beim Laden der Seite
       var storedTemperatureData = JSON.parse(localStorage.getItem('temperatureData')) || [];
       var storedHumidityData = JSON.parse(localStorage.getItem('humidityData')) || [];

       var storedTemperatureDataSPI = JSON.parse(localStorage.getItem('temperatureDataSPI')) || [];
       var storedHumidityDataSPI = JSON.parse(localStorage.getItem('humidityDataSPI')) || [];

       var storedAvgTemperatureData = JSON.parse(localStorage.getItem('avgTemperatureData')) || [];
       var storedAvgHumidityData = JSON.parse(localStorage.getItem('avgHumidityData')) || [];

  storedAvgTemperatureData.forEach(function(value, index) {
    updateChart(chartAvgOfAvgs, value, index);
  });

  storedAvgHumidityData.forEach(function(value, index) {
    updateChart(chartAvgOfAvgsHumidity, value, index);
  });

       // Fügt zuvor gespeicherte Daten zu den Diagrammen hinzu
       storedTemperatureData.forEach(function(value, index) {
         updateChart(chartTemperature, value, index);
       });

       storedHumidityData.forEach(function(value, index) {
         updateChart(chartHumidity, value, index);
       });

        storedTemperatureDataSPI.forEach(function(value) {
    updateChart(chartTemperatureSPI, value);
  });

  storedHumidityDataSPI.forEach(function(value) {
    updateChart(chartHumiditySPI, value);
  });
     });

     var avgTemperatureData = []; // Speichert die Durchschnittswerte

function storeAvgOfAvgs(temperature) {
  avgTemperatureData.push(temperature);
  if (avgTemperatureData.length > 30) {
    avgTemperatureData.shift(); // Entfernt den ältesten Wert, wenn mehr als 30 Werte vorhanden sind
  }
  updateAvgOfAvgsChart();
}

var avgHumidityData = []; // Speichert die Durchschnittswerte für Feuchtigkeit

function storeAvgOfAvgsHumidity(humidity) {
  avgHumidityData.push(humidity);
  if (avgHumidityData.length > 30) {
    avgHumidityData.shift(); // Entfernt den ältesten Wert, wenn mehr als 30 Werte vorhanden sind
  }
  updateAvgOfAvgsHumidityChart();
}



     function createChart(ctx, label, borderColor) {
       return new Chart(ctx, {
         type: 'line',
         data: {
           labels: [],
           datasets: [
             {
               label: label,
               borderColor: borderColor,
               data: [],
             },
           ],
         },
         options: {
           scales: {
             x: {
               type: 'linear',
               position: 'bottom',
             },
             y: {
               min: 0,
             },
           },
         },
       });
     }

function updateTemperatureChart(temperature) {
  updateChart(chartTemperature, temperature);
  saveDataLocally('temperatureData', temperature);

  if (chartTemperature.data.datasets[0].data.length >= 30) {
    var avg = chartTemperature.data.datasets[0].data.reduce((a, b) => a + b, 0) / 30;
    storeAvgOfAvgs(avg);
    chartTemperature.data.labels = [];
    chartTemperature.data.datasets[0].data = [];
    chartTemperature.update();
  }
}
function updateAvgOfAvgsChart() {
  updateChart(chartAvgOfAvgs, avgTemperatureData[avgTemperatureData.length - 1], avgTemperatureData.length - 1);
}

function updateHumidityChart(humidity) {
  updateChart(chartHumidity, humidity);
  saveDataLocally('humidityData', humidity);

  if (chartHumidity.data.datasets[0].data.length >= 30) {
    var avg = chartHumidity.data.datasets[0].data.reduce((a, b) => a + b, 0) / 30;
    storeAvgOfAvgsHumidity(avg);
    chartHumidity.data.labels = [];
    chartHumidity.data.datasets[0].data = [];
    chartHumidity.update();
  }
}

function updateAvgOfAvgsHumidityChart() {
  updateChart(chartAvgOfAvgsHumidity, avgHumidityData[avgHumidityData.length - 1], avgHumidityData.length - 1);
}
     function updateChart(chart, value, index) {
      var maxDataPoints = 30;
      var labels = chart.data.labels;
      var datasets = chart.data.datasets;
    
      if (labels.length >= maxDataPoints) {
        labels.shift();
        datasets[0].data.shift();
      }
    
      labels.push(index !== undefined ? index : labels.length);
      datasets[0].data.push(value);
    
      chart.update();
    }

     function saveDataLocally(key, value) {
       // Lade vorhandene Daten
       var storedData = JSON.parse(localStorage.getItem(key)) || [];
       // Füge den neuen Wert hinzu
       storedData.push(value);
       // Begrenze die Anzahl der gespeicherten Datenpunkte auf 30
       storedData = storedData.slice(-30);
       // Speichere die Daten lokal
       localStorage.setItem(key, JSON.stringify(storedData));
     }
     function storeAvgOfAvgs(temperature) {
  avgTemperatureData.push(temperature);
  if (avgTemperatureData.length > 30) {
    avgTemperatureData.shift(); // Entfernt den ältesten Wert, wenn mehr als 30 Werte vorhanden sind
  }
  localStorage.setItem('avgTemperatureData', JSON.stringify(avgTemperatureData)); // Speichern im LocalStorage
  updateAvgOfAvgsChart();
}

function storeAvgOfAvgsHumidity(humidity) {
  avgHumidityData.push(humidity);
  if (avgHumidityData.length > 30) {
    avgHumidityData.shift(); // Entfernt den ältesten Wert, wenn mehr als 30 Werte vorhanden sind
  }
  localStorage.setItem('avgHumidityData', JSON.stringify(avgHumidityData)); // Speichern im LocalStorage
  updateAvgOfAvgsHumidityChart();
}

     // Funktion zum Zurücksetzen der gespeicherten Daten
     function resetData() {
       localStorage.removeItem('temperatureData');
       localStorage.removeItem('humidityData');
       // Setze die Diagramme zurück
       chartTemperature.data.labels = [];
       chartTemperature.data.datasets[0].data = [];
       chartTemperature.update();

       chartHumidity.data.labels = [];
       chartHumidity.data.datasets[0].data = [];
       chartHumidity.update();

     }

     function resetChartData() {
  // Setzt die Daten der Diagramme zurück
  chartAvgOfAvgs.data.labels = [];
  chartAvgOfAvgs.data.datasets[0].data = [];
  chartAvgOfAvgs.update();

  chartAvgOfAvgsHumidity.data.labels = [];
  chartAvgOfAvgsHumidity.data.datasets[0].data = [];
  chartAvgOfAvgsHumidity.update();
}

function confirmAction(event, action) {
  var confirmed = confirm("Sind Sie sicher, dass Sie diese Aktion ausfuehren moechten?");
  if (confirmed) {
    // Führen Sie die Aktion aus, basierend auf dem übergebenen Aktionsnamen
    if (action === 'resetData') {
      resetData();
    } else if (action === 'resetChartData') {
      resetChartData();
    }
  } else {
    // Aktion abbrechen
    event.preventDefault();
  }
}
     source.addEventListener('avgTemperature', function(e) {
  console.log("avgTemperature", e.data);
  document.getElementById("avgTemp").innerHTML = e.data;
}, false);

source.addEventListener('avgHumidity', function(e) {
  console.log("avgHumidity", e.data);
  document.getElementById("avgHum").innerHTML = e.data;
}, false);

    }
  </script>
</body>
</html>)rawliteral";

void setup() {
  Serial.begin(9600);
  pinMode(fan_relay_gpio, OUTPUT);
  ledcSetup(fan_relay_gpio, 5000, PWM_RES);
  ledcAttachPin(fan_relay_gpio, PWM_CH_FAN);
  ledcWrite(PWM_CH_FAN, 0);

  WiFi.mode(WIFI_AP_STA);
  pinMode(fan_relay_gpio, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }
  if (!bmeSPI.begin()) {
    Serial.println("Konnte BME680-Sensor (SPI) nicht finden, überprüfen Sie die Verkabelung!");
    while (1);
  }
  if (!bmeSPI2.begin()) {
    Serial.println("Konnte BME680-Sensor (SPI2) nicht finden, überprüfen Sie die Verkabelung!");
    while (1);
  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);

  bmeSPI.setTemperatureOversampling(BME680_OS_8X);
  bmeSPI.setHumidityOversampling(BME680_OS_2X);

  bmeSPI2.setTemperatureOversampling(BME680_OS_8X);
  bmeSPI2.setHumidityOversampling(BME680_OS_2X);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  events.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });

  server.on("/fan_on", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("ON");
    digitalWrite(fan_relay_gpio, HIGH);
    request->send(200, "text/plain", "Fan turned ON, bitte zurueckkehren");
  });

  server.on("/fan_off", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("OFF");
    digitalWrite(fan_relay_gpio, LOW);
    request->send(200, "text/plain", "Fan turned OFF, bitte zurueckkehren");
  });

  server.addHandler(&events);
  server.begin();
}

void loop() {
  if (Serial.available() > 0) {
    modi = Serial.readStringUntil('\n');
    Serial.println(modi);
    if (modi.equals("ON")) {
      Serial.println("Before ledcWrite");
      ledcWrite(PWM_CH_FAN, 255);
      Serial.println("After ledcWrite");
    }
    else if (modi.equals("OFF")) {
      ledcWrite(PWM_CH_FAN, 0);
    }
  }
  if ((millis() - lastTime) > timerDelay) {
    getBME680Readings();
    calculateAverages();

    Serial.printf("Temperature = %.2f ºC \n", temperature);
    Serial.printf("Humidity = %.2f % \n", humidity);
    Serial.printf("Pressure = %.2f hPa \n", pressure);
    Serial.printf("Gas Resistance = %.2f KOhm \n", gasResistance);
    Serial.println();
    Serial.println("TEMP SPI2");
    Serial.println(bmeSPI2.temperature);

    events.send("ping", NULL, millis());
    events.send(String(temperature).c_str(), "temperature", millis());
    events.send(String(humidity).c_str(), "humidity", millis());
    events.send(String(pressure).c_str(), "pressure", millis());
    events.send(String(gasResistance).c_str(), "gas", millis());
    events.send(String(temperatureSPI).c_str(), "temperatureSPI", millis());
    events.send(String(humiditySPI).c_str(), "humiditySPI", millis());
    events.send(String(temperatureSPI2).c_str(), "temperatureSPI2", millis());
    events.send(String(humiditySPI2).c_str(), "humiditySPI2", millis());


    lastTime = millis();
  }
}
