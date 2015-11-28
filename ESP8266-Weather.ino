#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "Adafruit_BMP085.h"
#include "settings.h"

Adafruit_BMP085 bmp;
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to network: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(WiFi.localIP());
  if (!bmp.begin()) {
    Serial.println("Can't initialize sensor");
    while(1);
  }
}

void loop() {
    float temp = bmp.readTemperature();
    float press = bmp.readPressure() / 100;

    String url = String(DATA_PATH) + "&temp=" + String(temp) + "&pressure=" + String(press);
    WiFiClientSecure client;

    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print(" Pressure: ");
    Serial.println(press);
    
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
    delay(300000);
}
