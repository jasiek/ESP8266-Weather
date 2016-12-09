#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SparkFunHTU21D.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <FS.h>

ADC_MODE(ADC_VCC);
HTU21D htu;
DHT dht(D5, DHT11);
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
PubSubClient mqtt(client);

String WIFI_SSID;
String WIFI_PASS;
String MQTT_SERVER;
int MQTT_PORT;

void readConfiguration() {
  StaticJsonBuffer<200> buffer;
  char readFileBuffer[200];
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS could not be accessed");
    return;
  }

  File f = SPIFFS.open("/config.json", "r");

  if (!f) {
    Serial.println("Opening config.json failed");
    return;
  }

  Serial.println(f.readBytes(readFileBuffer, 200));
  Serial.println(readFileBuffer);
  JsonObject &root = buffer.parse(readFileBuffer);

  if (!root.success()) {
    Serial.println("Parsing config.json failed");
    return;
  }

  WIFI_SSID = root["wifi_ssid"].asString();
  WIFI_PASS = root["wifi_pass"].asString();
  MQTT_SERVER = root["mqtt_server"].asString();
  MQTT_PORT = root["mqtt_port"].as<int>();

  SPIFFS.end();
}

void setup() {
  Serial.println();
  Serial.begin(115200);
  Serial.println("ESP8266 Weather Station");
  Serial.flush();

  readConfiguration();

  WiFiMulti.addAP(WIFI_SSID.c_str(), WIFI_PASS.c_str());
  maybeReconnect();

  mqtt.setServer(MQTT_SERVER.c_str(), MQTT_PORT);
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
  StaticJsonBuffer<200> buffer;
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

  String stream;
  JsonObject& root = buffer.createObject();
  root["temperature"] = temp;
  root["humidity"] = humid;
  root["voltage"] = vcc;
  root["rssi"] = rssi;
  root.printTo(stream);
  Serial.println(stream);

  if (mqtt.connect("ESP8266-Weather")) {
    String topic = "/devices/" + nodeName();
    mqtt.publish(topic.c_str(), stream.c_str(), true);
    mqtt.disconnect();
  }
}

void loop() {
  // NOOP
}
