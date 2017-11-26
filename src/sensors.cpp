#include "sensors.h"

#include <DHT.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_HTU21DF.h>

Adafruit_HTU21DF htu;
Adafruit_BMP085 bmp;
DHT dht(D5, DHT11);

enum SensorType installedSensor;

enum SensorType sensors::detect() {
  if (installedSensor != SK_NONE) return installedSensor;

  Wire.begin(D2, D1);
  if (htu.begin()) {
    return installedSensor = SK_HTU21D;
  }

  Wire.begin(D6, D5);
  if (bmp.begin()) {
    return installedSensor = SK_BMP180;
  }

  dht.begin();
  if (!isnan(dht.readTemperature())) {
    return installedSensor = SK_DHT11;
  }

  return installedSensor = SK_NONE;
}

float sensors::readTemperature() {
  switch(installedSensor) {
    case SK_HTU21D:
      return htu.readTemperature();
    case SK_BMP180:
      return bmp.readTemperature();
    default:
      return dht.readTemperature();
  }
}

float sensors::readHumidity() {
  switch(installedSensor) {
    case SK_HTU21D:
      return htu.readHumidity();
    case SK_BMP180:
      return NAN;
    default:
      return dht.readHumidity();
  }
}

float sensors::readPressure() {
  switch(installedSensor) {
    case SK_BMP180:
      return bmp.readPressure() / 100;
    default:
      return NAN;
  }
}
