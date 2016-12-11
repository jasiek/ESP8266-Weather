#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SparkFunHTU21D.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <SPI.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_HTU21DF.h>
#include <FS.h>

ADC_MODE(ADC_VCC);

Adafruit_HTU21DF htu;
Adafruit_BMP085 bmp;
DHT dht(D5, DHT11);

ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
PubSubClient mqtt(client);

String WIFI_SSID;
String WIFI_PASS;
String MQTT_SERVER;
int MQTT_PORT;

typedef enum SensorType {
  SK_NONE = 0,
  SK_HTU21D,
  SK_DHT11,
  SK_BMP180
} SensorType;

SensorType installedSensor;

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
  Serial.println();
  Serial.begin(115200);
  Serial.println("ESP8266 Weather Station");
  Serial.flush();

  readConfiguration();

  WiFiMulti.addAP(WIFI_SSID.c_str(), WIFI_PASS.c_str());
  maybeReconnect();

  mqtt.setServer(MQTT_SERVER.c_str(), MQTT_PORT);

  Wire.begin(D2, D1);
  installedSensor = determineSensorType();
  Serial.println(installedSensor);

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

void report() {
  if (installedSensor == SK_NONE) {
    Serial.println("No sensor detected.");
    return;
  }

  StaticJsonBuffer<200> buffer;
  float temp = readTemperature();
  Serial.println(temp);
  float humid = readHumidity();
  Serial.println(humid);
  float press = readPressure();
  Serial.println(press);
  float vcc = ESP.getVcc() / 1000.0;
  int rssi = WiFi.RSSI();

  String stream;
  JsonObject& root = buffer.createObject();
  if (!isnan(temp)) {
    root["temperature"] = temp;
  }
  if (!isnan(humid)) {
    root["humidity"] = humid;
  }
  if (!isnan(press)) {
    root["pressure"] = press;
  }

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
