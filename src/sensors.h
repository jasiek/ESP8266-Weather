#ifndef _SENSORS_H
#define _SENSORS_H

enum SensorType {
  SK_NONE = 0,
  SK_HTU21D,
  SK_DHT11,
  SK_BMP180
};

namespace sensors {
  enum SensorType detect();
  float readTemperature();
  float readHumidity();
  float readPressure();
}

#endif
