#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <metering.h>
#include "sensors.h"

Ticker resetter;

void restart() {
  ESP.restart();
}

void report() {
  DynamicJsonBuffer buffer;
  String stream;
  JsonObject& root = buffer.createObject();

  float temp = sensors::readTemperature();
  if (!isnan(temp)) {
    root["temperature"] = temp;
  }

  float humidity = sensors::readHumidity();
  if (!isnan(humidity)) {
    root["humidity"] = humidity;
  }

  float pressure = sensors::readPressure();
  if (!isnan(pressure)) {
    root["pressure"] = pressure;
  }

  root["rssi"] = WiFi.RSSI();
  uint32 timestamp = m_time::get_current();
  if (timestamp > 0) root["timestamp"] = timestamp;
  root.printTo(stream);

  network::report(stream);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("ESP8266 Weather Station");
  Serial.print("Built on ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.print(__TIME__);
  Serial.print(" from ");
  Serial.println(GIT_REVISION);
  Serial.println("-----------------------");
  Serial.flush();

  if (sensors::detect() == SK_NONE) {
    Serial.println("No sensor installed.");
    delay(3000);
    ESP.restart();
  }

  network::start("ESP8266-Weather");
  // Wait for 10 seconds for WiFi to settle
  delay(10000);
  network::hello();
  report();
  updater::begin(&resetter);
  resetter.once(300, restart);
}

void loop() {
  network::loop();
  delay(1000);
}
