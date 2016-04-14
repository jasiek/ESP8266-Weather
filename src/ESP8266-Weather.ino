#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include <Adafruit_HTU21DF.h>
#include "settings.h"

ADC_MODE(ADC_VCC);
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP8266 Weather Station");
  Serial.flush();
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  maybeReconnect();
}

void maybeReconnect() {
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println("(Re)connecting...");
    delay(1000);
  }
  Serial.print("connected, got IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
   if (!htu.begin()) {
     Serial.println("Can't initialize sensor");
     while(1);
   }

    float temp = htu.readTemperature();
    float press = htu.readHumidity();
    float vcc = ESP.getVcc() / 1000.0;
    int rssi = WiFi.RSSI();

    Serial.print("Temp: ");
    Serial.print(temp);
    Serial.print(" Pressure: ");
    Serial.print(press);
    Serial.print(" Voltage: ");
    Serial.print(vcc);
    Serial.print(" RSSI: ");
    Serial.println(rssi);

    ESP.deepSleep(600 * 1000000);
    delay(1000);
}
