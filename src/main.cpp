#include <Arduino.h>
#include <SparkFunHTU21D.h>
#include <DHT.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_HTU21DF.h>

#include "network.h"

ADC_MODE(ADC_VCC);

Adafruit_HTU21DF htu;
Adafruit_BMP085 bmp;
DHT dht(D5, DHT11);

typedef enum SensorType {
  SK_NONE = 0,
  SK_HTU21D,
  SK_DHT11,
  SK_BMP180
} SensorType;

SensorType installedSensor;

float readTemperature() {
  switch(installedSensor) {
    case SK_HTU21D:
      return htu.readTemperature();
    case SK_BMP180:
      return bmp.readTemperature();
    default:
      dht.readTemperature();
  }
}

float readHumidity() {
  switch(installedSensor) {
    case SK_HTU21D:
      return htu.readHumidity();
    case SK_BMP180:
      return NAN;
    default:
      return dht.readHumidity();
  }
}

float readPressure() {
  switch(installedSensor) {
    case SK_BMP180:
      return bmp.readPressure() / 100;
    default:
      return NAN;
  }
}


SensorType determineSensorType() {
  if (htu.begin()) {
    return SK_HTU21D;
  }
  if (bmp.begin()) {
    return SK_BMP180;
  }

  dht.begin();
  if (!isnan(dht.readTemperature())) {
    return SK_DHT11;
  }

  return SK_NONE;
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

  Wire.begin(D2, D1);
  installedSensor = determineSensorType();
  Serial.println(installedSensor);
  if (installedSensor == SK_NONE) {
    Serial.println("No sensor installed.");
    delay(3000);
    ESP.reset();
  }

  network::start();
  network::report(readTemperature(),
    readHumidity(),
    readPressure(),
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
