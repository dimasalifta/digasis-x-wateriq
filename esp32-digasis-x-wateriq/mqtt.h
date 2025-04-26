#include <PubSubClient.h>
#include <ArduinoJson.h>
// MQTT details
const char* broker = "digitalasistensi.com";
const char* topic       = "bot";
const char* topicInit      = "bot/stasiun_cuaca/init";
const char* topicData      = "bot/stasiun_cuaca";

const int jsonsize = 512;
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


//void sendToMQTT(String timestampp, float temperature, float humidity,
//                float load_voltage, float load_current, float load_power,
//                float battery_voltage, float battery_current, float battery_power,
//                float solar_voltage, float solar_current, float solar_power,
//                float water_temperature) {
//
//  if (solar_current < 0) solar_current = 0.0;
//  if (battery_current < 0) battery_current = 0.0;
//  if (load_current < 0) load_current = 0.0;
//  // ✅ Ukuran cukup besar untuk seluruh struktur
//  DynamicJsonDocument jsonDoc(1024);
//
//  jsonDoc["device_id"] = "DIGASIS-01";
//  jsonDoc["device_type"] = "ESP32";
//  jsonDoc["location"] = "Kebon Kopi";
//  jsonDoc["timestamp"] = timestampp;
//
//  JsonObject panel_status = jsonDoc.createNestedObject("panel_status");
//  panel_status["temperature"] = temperature;
//  panel_status["humidity"] = humidity;
//  //
//  JsonObject spc_status = jsonDoc.createNestedObject("spc_status");
//  //
////  spc_status["solar_voltage"] = solar_voltage;
//  //  spc_status["solar_current"] = solar_current;
//  //  spc_status["solar_power"] = solar_power;
//  //
//  //  spc_status["battery_voltage"] = battery_voltage;
//  //  spc_status["battery_current"] = battery_current;
//  //  spc_status["battery_power"] = battery_power;
//  //
//  //  spc_status["load_voltage"] = load_voltage;
//  //  spc_status["load_current"] = load_current;
//  //  spc_status["load_power"] = load_power;
//
//  JsonObject env_status = jsonDoc.createNestedObject("env_status");
//  env_status["water_temperature"] = water_temperature;
//
//  // ✅ Ukur panjang JSON dan alokasikan buffer dengan margin
//  size_t jsonSize = measureJson(jsonDoc);
//  SerialMon.print("Ukuran JSON (bytes): ");
//  SerialMon.println(jsonSize);
//
//  // ✅ Alokasikan buffer output yang cukup besar
//  const size_t bufferSize = jsonSize + 64;  // Tambah margin
//  char buffer[bufferSize];
//  serializeJson(jsonDoc, buffer, sizeof(buffer));
//
//  // Debug: print isi JSON
//  SerialMon.println("Payload JSON:");
//  SerialMon.println(buffer);
//
//  // ✅ Publish ke MQTT
//  if (mqtt.publish(topicData, buffer)) {
//    SerialMon.println("✅ MQTT Publish Success");
//  } else {
//    SerialMon.println("❌ MQTT Publish Failed");
//  }
//}

void sendToMQTT(String timestampp, float temperature, float humidity,
                float load_voltage, float load_current, float load_power,
                float battery_voltage, float battery_current, float battery_power,
                float solar_voltage, float solar_current, float solar_power,
                float water_temperature) {

//  // Optional: normalisasi nilai aneh
//  if (battery_current < 0) {
//    battery_current = 0.00;
//  }

  // Format CSV: device_id,device_type,location,timestamp,temperature,humidity,...
  String payload = "DIGASIS-01,ESP32,KebonKopi," + timestampp + "," +
                   String(temperature, 2) + "," +
                   String(humidity, 2) + "," +
                   String(solar_voltage, 2) + "," +
                   String(solar_current, 2) + "," +
                   String(solar_power, 2) + "," +
                   String(battery_voltage, 2) + "," +
                   String(battery_current, 2) + "," +
                   String(battery_power, 2) + "," +
                   String(load_voltage, 2) + "," +
                   String(load_current, 2) + "," +
                   String(load_power, 2) + "," +
                   String(water_temperature, 2);

  // Cetak untuk debug
  SerialMon.print("CSV Payload: ");
  SerialMon.println(payload);

  // Kirim via MQTT
  if (mqtt.publish(topicData, payload.c_str())) {
    SerialMon.println("✅ MQTT CSV Publish Success");
  } else {
    SerialMon.println("❌ MQTT CSV Publish Failed");
  }
}
