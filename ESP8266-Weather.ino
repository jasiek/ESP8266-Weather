#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "Adafruit_BMP085.h"
#include "settings.h"

Adafruit_BMP085 bmp;

ADC_MODE(ADC_VCC);

void setup() {

}

void loop() {
    Serial.begin(115200);
    Serial.println();
  
   if (!bmp.begin()) {
     Serial.println("Can't initialize sensor");
     while(1);
   }

    Serial.print("Connecting to network: ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println(WiFi.localIP());

    float temp = bmp.readTemperature();
    float press = bmp.readPressure() / 100;
    float vcc = ESP.getVcc() / 1000.0;

    String url = String(DATA_PATH) + "&temp=" + String(temp) + "&pressure=" + String(press) + "&vcc=" + String(vcc);
    WiFiClientSecure client;

    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print(" Pressure: ");
    Serial.print(press);
    Serial.print(" Voltage: ");
    Serial.println(vcc); 
    
    if (client.connect(DATA_HOST, DATA_PORT)) {
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
      "Host: data.sparkfun.com\r\n" +
      "User-Agent: ESP8266-Weather\r\n" +
      "Connection: close\r\n\r\n");
      Serial.println("Sent request");

      bool success = false;

      while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line.startsWith("1 success")) {
          success = true;
          break;
        }
      }
      success ? Serial.println("Data sent successfully") : Serial.println("An error occured");
    } else {
      Serial.println("Could not connect, will try later");
    }
    ESP.deepSleep(600 * 1000000);
    delay(1000);
}
