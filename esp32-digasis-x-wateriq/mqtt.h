#include <PubSubClient.h>
#include <ArduinoJson.h>
// MQTT details
const char* broker = "digitalasistensi.com";
const char* topic       = "bot";
const char* topicInit      = "bot/stasiun_cuaca/init";
const char* topicData      = "bot/stasiun_cuaca";

uint32_t lastReconnectAttempt = 0;
const unsigned long mqttInterval = 300000; // Interval pengiriman dalam milidetik (5 detik)
unsigned long lastMqttSend = 0;
PubSubClient  mqtt(client);

boolean mqttConnect() {
  SerialMon.print("Connecting to MQTT broker...");
  boolean status = mqtt.connect("StasiunCuaca");

  if (!status) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  mqtt.publish(topicInit, "{\"message\": \"StasiunCuaca started\"}");
  return mqtt.connected();

}


void mqtt_setup() {
  //  SerialMon.begin(115200);
  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  //  mqtt.setCallback(mqttCallback);
}


void sendToMQTT(float temperature, float humidity,  float load_voltage,  float load_current, float load_power, float battery_voltage,  float battery_current, float battery_power, float solar_voltage,  float solar_current, float solar_power) {
  DynamicJsonDocument jsonDoc(256);
  jsonDoc["device_id"] = "DIGASIS-02";
  jsonDoc["device_type"] = "ESP32";
  jsonDoc["location"] = "Serpong Utara";
  jsonDoc["timestamp"] = "2025-02-14T12:34:56Z";

  JsonObject data = jsonDoc.createNestedObject("data");
  data["temperature"] = temperature;
  data["humidity"] = humidity;

  JsonObject Battery_status = jsonDoc.createNestedObject("Battery_status");
  if (battery_current < 0) {
    battery_current = 0.00;  // Atasi nilai negatif
  }
  Battery_status["voltage"] = battery_voltage;
  Battery_status["current"] = battery_current;
  Battery_status["power"] = battery_power;
  jsonDoc["battery_level"] = 69;

  char buffer[256];
  serializeJson(jsonDoc, buffer);

  if (mqtt.publish(topicData, buffer)) {
    SerialMon.println("MQTT Publish Success");
  } else {
    SerialMon.println("MQTT Publish Failed");
  }
}
