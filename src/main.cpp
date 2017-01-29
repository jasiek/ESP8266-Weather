#include <Arduino.h>
#include "network.h"
#include "sensors.h"

ADC_MODE(ADC_VCC);

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

  delay(300 * 1000);
  ESP.restart();
}

void loop() {
  // NOOP
}

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  Serial.print("incoming: ");
  Serial.print(topic);
  Serial.print(" - ");
  Serial.print(payload);
  Serial.println();
}
