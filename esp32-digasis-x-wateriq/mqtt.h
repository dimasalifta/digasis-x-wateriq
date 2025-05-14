#include <PubSubClient.h>
#include <ArduinoJson.h>
// MQTT details
const char* broker = "103.127.138.196";
const char* topic       = "bot";
const char* topicInit      = "bot/stasiun_cuaca/init";
const char* topicData      = "bot/stasiun_cuaca";

uint32_t lastReconnectAttempt = 0;
const unsigned long mqttInterval = 300000; // Interval pengiriman dalam milidetik (5 detik)
unsigned long lastMqttSend = 0;
PubSubClient  mqtt(client);

boolean mqttConnect() {
  Serial.print("Connecting to MQTT broker...");
  boolean status = mqtt.connect("StasiunCuaca");

  if (!status) {
    Serial.println(" fail");
    return false;
  }
  Serial.println(" success");
  mqtt.publish(topicInit, "{\"message\": \"StasiunCuaca started\"}");
  return mqtt.connected();

}


void mqtt_setup() {
//  Serial.begin(115200);
  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  //  mqtt.setCallback(mqttCallback);
}


void sendToMQTT(float temperature, float humidity,  float voltagee,  float currentt, float powerr) {
  DynamicJsonDocument jsonDoc(256);
  jsonDoc["device_id"] = "DIGASIS-02";
  jsonDoc["device_type"] = "ESP32";
  jsonDoc["location"] = "Serpong Utara";
  jsonDoc["timestamp"] = "2025-02-14T12:34:56Z";

  JsonObject data = jsonDoc.createNestedObject("data");
  data["temperature"] = temperature;
  data["humidity"] = humidity;

  JsonObject Battery_status = jsonDoc.createNestedObject("Battery_status");
  if (currentt < 0) {
    currentt = 0.00;  // Atasi nilai negatif
  }
  Battery_status["voltage"] = voltagee;
  Battery_status["current"] = currentt;
  Battery_status["power"] = powerr;
  jsonDoc["battery_level"] = 69;

  char buffer[256];
  serializeJson(jsonDoc, buffer);

  if (mqtt.publish(topicData, buffer)) {
    Serial.println("MQTT Publish Success");
  } else {
    Serial.println("MQTT Publish Failed");
  }
}
