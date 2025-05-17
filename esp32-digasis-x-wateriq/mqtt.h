#include <PubSubClient.h>
#include <ArduinoJson.h>
// MQTT details
const char* broker = "iot.digitalasistensi.com";
const char* topic       = "bot";
const char* topicInit      = "bot/stasiun_cuaca/init";
const char* topicData      = "bot/stasiun_cuaca";

const int jsonsize = 512;
uint32_t lastReconnectAttempt = 0;
const unsigned long mqttInterval = 300000;
unsigned long lastMqttSend = 0;
PubSubClient  mqtt(client);


const char* mqttStateString(int state) {
  switch (state) {
    case -4: return "Connection timeout";
    case -3: return "Connection lost";
    case -2: return "Connect failed";
    case -1: return "Disconnected";
    case 0:  return "Connected";
    case 1:  return "Bad protocol";
    case 2:  return "Bad client ID";
    case 3:  return "Unavailable";
    case 4:  return "Bad credentials";
    case 5:  return "Unauthorized";
    default: return "Unknown error";
  }
}


boolean mqttConnect() {
  SerialMon.print("Connecting to MQTT broker...");

  // Client ID harus unik jika ada banyak device
  String clientId = "StasiunCuaca-fadhel";

  // Jika menggunakan username/password, gunakan versi ini:
  // boolean status = mqtt.connect(clientId.c_str(), "mqtt_user", "mqtt_pass");

  boolean status = mqtt.connect(clientId.c_str());

  if (!status) {
    SerialMon.println(" fail (mqtt)");
    SerialMon.print("MQTT connect failed, state: ");
    SerialMon.println(mqtt.state());
    SerialMon.print("Reason: ");
    SerialMon.println(mqttStateString(mqtt.state()));  // Gunakan fungsi yang saya beri sebelumnya
    return false;
  }

  SerialMon.println(" success");

  mqtt.setKeepAlive(60); // Keep alive 60 detik — ini disarankan ditaruh SEBELUM mqtt.connect()
  
  // Publish pesan awal
  mqtt.publish(topicInit, "{\"message\": \"StasiunCuaca started-fadhel\"}");

  return mqtt.connected();
}



void mqtt_setup() {
  //  SerialMon.begin(115200);
  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  //  mqtt.setCallback(mqttCallback);
}

void sendToMQTT(String timestampp, float temperature, float humidity,
                float load_voltage, float load_current, float load_power,
                float battery_voltage, float battery_current, float battery_power,
                float solar_voltage, float solar_current, float solar_power,
                float water_temperature, float flow_meter) {


  // Format CSV: device_id,device_type,location,timestamp,temperature,humidity,...
  String payload = "DIGASIS-01,ESP32,Kebon-Kopi," + timestampp + "," +
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
                   String(water_temperature, 2)+ "," +
                   String(flow_meter, 2)
                   ;

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
