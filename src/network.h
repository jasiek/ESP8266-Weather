#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <MQTTClient.h>
#include "debug.h"

struct network_config_t {
  bool ok;
  char wifi_ssid[128];
  char wifi_pass[128];
  char mqtt_server[128];
  int mqtt_port;
  bool mqtt_ssl;
  char mqtt_username[128];
  char mqtt_password[128];
  char node_name[8];
};

namespace network {
  void start();
  network_config_t *config();
  void report(float temp, float humidity, float pressure, float vcc);
  const char* mqtt_client_name();
  const char* mqtt_topic();
  void maybe_reconnect();
}

#endif
