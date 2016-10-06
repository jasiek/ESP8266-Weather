#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <SparkFunHTU21D.h>
#include <PubSubClient.h>
#include <DHT.h>
#include "settings.h"

ADC_MODE(ADC_VCC);
HTU21D htu;
DHT dht(D5, DHT11);
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
PubSubClient mqtt(client);

void setup() {
  Serial.println();
  Serial.begin(115200);
  Serial.println("ESP8266 Weather Station");
  Serial.flush();
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  maybeReconnect();

  mqtt.setServer("192.168.1.2", 1883);
  dht.begin();
  htu.begin();

  report();

  delay(300 * 1000);
  ESP.restart();
}

String nodeName() {
  String nodeName = WiFi.macAddress();
  for (int i = nodeName.indexOf(':'); i > -1; i = nodeName.indexOf(':')) nodeName.remove(i, 1);
  nodeName.toLowerCase();
  return nodeName;
}

void maybeReconnect() {
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println("(Re)connecting...");
    delay(1000);
  }
  Serial.print("connected, got IP: ");
  Serial.println(WiFi.localIP());
}

float readTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    t = htu.readTemperature();
  }
  return t;
}

float readHumidity() {
  float t = dht.readHumidity();
  if (isnan(t)) {
    t = htu.readHumidity();
  }
  return t;
}

void report() {
  float temp = readTemperature();
  float humid = readHumidity();
  float vcc = ESP.getVcc() / 1000.0;
  int rssi = WiFi.RSSI();

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" Humidity: ");
  Serial.print(humid);
  Serial.print(" Voltage: ");
  Serial.print(vcc);
  Serial.print(" RSSI: ");
  Serial.println(rssi);

  HTTPClient http;

  String url = String(API_ENDPOINT) + nodeName();
  Serial.println("POST: " + url);

  http.begin(url);
  String stream;
  StaticJsonBuffer<200> buffer;
  JsonObject& root = buffer.createObject();
  root["temperature"] = temp;
  root["humidity"] = humid;
  root["voltage"] = vcc;
  root["rssi"] = rssi;
  root.printTo(stream);
  Serial.println(stream);

  int status = http.POST(stream);
  if (status == 200) {
    Serial.println("...success!");
  } else {
    Serial.println(status);
    Serial.println(http.getString());
  }
  Serial.println("end");

  http.end();

  if (mqtt.connect("ESP8266-Weather")) {
    String topic = "/devices/" + nodeName();
    mqtt.publish(topic.c_str(), stream.c_str());
    mqtt.disconnect();
  }
}

void loop() {
  // NOOP
}
