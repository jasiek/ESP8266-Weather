#include "network.h"
#include <FS.h>
#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;
WiFiClient clientRegular;
WiFiClientSecure clientSecure;
MQTTClient mqtt;
network_config_t network_config;

void network::start() {
  WiFi.mode(WIFI_STA);

  String nodeName = WiFi.macAddress();
  for (int i = nodeName.indexOf(':'); i > -1; i = nodeName.indexOf(':')) nodeName.remove(i, 1);
  nodeName.toLowerCase();
  strncpy(network_config.node_name, nodeName.c_str(), 12);
  DEBUG("Node name: ");
  DEBUG(network_config.node_name);

  WiFiMulti.addAP(network::config()->wifi_ssid, network::config()->wifi_pass);
}

network_config_t *network::config() {
  if (network_config.ok) return &network_config;

  StaticJsonBuffer<200> buffer;
  char readFileBuffer[200];
  if (!SPIFFS.begin()) {
    DEBUG("SPIFFS could not be accessed");
    network_config.ok = false;
    return &network_config;
  }

  File f = SPIFFS.open("/config.json", "r");

  if (!f) {
    DEBUG("Opening config.json failed");
    network_config.ok = false;
    return &network_config;
  }

  f.readBytes(readFileBuffer, 200);
  DEBUG(readFileBuffer);
  JsonObject &root = buffer.parse(readFileBuffer);

  if (!root.success()) {
    DEBUG("Parsing config.json failed");
    network_config.ok = false;
    return &network_config;
  }

  strncpy(network_config.wifi_ssid, root["wifi_ssid"].asString(), 128);
  strncpy(network_config.wifi_pass, root["wifi_pass"].asString(), 128);
  strncpy(network_config.mqtt_server, root["mqtt_server"].asString(), 128);
  network_config.mqtt_port = root["mqtt_port"].as<int>();
  strncpy(network_config.mqtt_username, root["mqtt_username"].asString(), 128);
  strncpy(network_config.mqtt_password, root["mqtt_password"].asString(), 128);
  network_config.mqtt_ssl = root["mqtt_ssl"].as<bool>();
  network_config.ok = true;

  SPIFFS.end();

  return &network_config;
}

void network::report(float temp, float humidity, float pressure, float vcc) {
  maybe_reconnect();

  StaticJsonBuffer<200> buffer;
  String stream;
  JsonObject& root = buffer.createObject();
  if (!isnan(temp)) {
    root["temperature"] = temp;
  }
  if (!isnan(humidity)) {
    root["humidity"] = humidity;
  }
  if (!isnan(pressure)) {
    root["pressure"] = pressure;
  }

  root["voltage"] = vcc;
  root["rssi"] = WiFi.RSSI();
  root.printTo(stream);
  DEBUG(stream);

  DEBUG("sending");
  mqtt.begin(network_config.mqtt_server,
    network_config.mqtt_port,
    network_config.mqtt_ssl ? clientSecure : clientRegular);

  if (mqtt.connect(mqtt_client_name(), network_config.mqtt_username, network_config.mqtt_password)) {

    MQTTMessage message;
    message.topic = (char*)mqtt_topic();
    message.length = stream.length();
    message.payload = (char *)stream.c_str();
    message.retained = true;

    if (mqtt.publish(&message)) {
      Serial.println("published");
    }

    mqtt.disconnect();
  }
}

const char *network::mqtt_client_name() {
  static char client_name[64];
  sprintf(client_name, "ESP8266-Weather (%s)", network_config.node_name);
  return client_name;
}

const char *network::mqtt_topic() {
  static char topic[64];
  sprintf(topic, "/devices/%s", network_config.node_name);
  return topic;
}

void network::maybe_reconnect() {
  WiFi.persistent(false);

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.println("(Re)connecting...");
    delay(1000);
  }

  DEBUG("connected, got IP: ");
  DEBUG(WiFi.localIP());
}
