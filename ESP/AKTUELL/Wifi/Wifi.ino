
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <String.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

unsigned long previousMillisFan = 0;
unsigned long previousMillisPrint = 0;
const long intervalFan = 100;
const long intervalPrint = 5000;

Adafruit_BME680 bme; // I2C

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
#define FAN1        25
#define PWM_CH_FAN  1
#define PWM_RES     8
String modi = "";
int PWM1_DutyCycle = 0;


void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}
WiFiUDP Udp;
unsigned int localPort = 9999;      // Portnummer
String packet_Temp;
String packet_Humidity;


void setup() {
  Serial.begin(9600);
  ledcSetup(PWM_CH_FAN, 5000, PWM_RES);
  ledcAttachPin(FAN1, PWM_CH_FAN);
  while (!Serial);
  Serial.println(F("BME680 async test"));

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  initWiFi();
  Serial.print("RRSI: ");
  Serial.println(WiFi.RSSI());
  printWifiStatus();
}

void loop() {
  unsigned long currentMillisFan = millis();
  unsigned long currentMillisPrint = millis();
  if (currentMillisFan - previousMillisFan >= intervalFan){
    previousMillisFan = currentMillisFan; 
    if (Serial.available() > 0) {
    modi = Serial.readStringUntil('\n');
    Serial.println(modi);
    if (modi.equals("on")) {
    ledcWrite(PWM_CH_FAN, 255);

    }
    else if (modi.equals("middle")) {
    ledcWrite(PWM_CH_FAN, 125);

    }
    else {
      ledcWrite(PWM_CH_FAN, 0);

    }
  }
  }

   if (currentMillisPrint - previousMillisPrint >= intervalPrint){
    previousMillisPrint = currentMillisPrint;
    // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  Serial.println(F("You can do other work during BME680 measurement."));
  delay(50); // This represents parallel work.
  
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  Serial.print(F("Temperature = "));
  Serial.print(bme.temperature);
  Serial.println(F(" *C"));

  Serial.print(F("Humidity = "));
  Serial.print(bme.humidity);
  Serial.println(F(" %"));

  packet_Temp = String(bme.temperature); 
  packet_Humidity = String(bme.humidity);

    //##Packet senden##
    Udp.beginPacket("255.255.255.255", localPort);     // hier die IP-Adresse vom "IPTest.ino" festlegen
    Udp.print(packet_Temp + "!" + packet_Humidity);
    Serial.println(packet_Temp + "!" + packet_Humidity);
    Serial.println("Packet Temperatur gesendet");
    Udp.endPacket();

    
  Serial.println();
  
  }
}
  
   



void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
