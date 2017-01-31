#include <Arduino.h>
#include <Ticker.h>
#include "network.h"
#include "sensors.h"
#include "updater.h"

ADC_MODE(ADC_VCC);

Ticker resetter;

void restart() {
  ESP.restart();
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

  network::start();
  network::report(sensors::readTemperature(),
    sensors::readHumidity(),
    sensors::readPressure(),
    ESP.getVcc() / 1000.0);
  updater::begin(&resetter);
  resetter.once(300, restart);
}

void loop() {
  network::loop();
  delay(1000);
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  network::mqtt_message_received_cb(topic, payload, bytes, length);
}
