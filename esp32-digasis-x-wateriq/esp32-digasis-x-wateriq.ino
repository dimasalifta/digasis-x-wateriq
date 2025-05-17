#include "sim800lv2.h"
#include "mqtt.h"
#include "rs485_xymd02.h"
#include "i2c_ina219.h"
#include "ds18b20.h"
#include "flowmeter.h"
#include "motor.h"
#define MODEM_RST 15

int extractHourFromGSMTime(String timeStr) {
  // timeStr: "25/05/17,15:43:40+28"
  int commaIndex = timeStr.indexOf(',');
  String timePart = timeStr.substring(commaIndex + 1, commaIndex + 3); // Ambil jam (HH)
  return timePart.toInt();
}

void hardwareResetModem() {
  SerialMon.println("== HARDWARE RESET MODEM ==");
  digitalWrite(MODEM_RST, LOW);
  delay(1000); // tahan 1 detik
  digitalWrite(MODEM_RST, HIGH);
  delay(5000); // tunggu modem bangun
  SerialMon.println("== HARDWARE RESET MODEM SUCCESS ==");
  sim800lv2_setup();
}


void setup() {
  SerialMon.begin(115200);
  delay(2000);
  pinMode(MODEM_RST, OUTPUT);
  digitalWrite(MODEM_RST, HIGH); // pastikan awalnya tidak di-reset

  sim800lv2_setup();
  mqtt_setup();
  rs485_xymd02_setup();
  i2c_ina219_setup();
  ds18b20_setup();
  watersetup();
  motor_setup();
  SerialMon.println("DEVICE READY!!!");

}

void loop() {
  static int mqttFailCount = 0;
  static unsigned long lastCheck = 0;

  // Cek koneksi jaringan setiap 10 detik
  if (millis() - lastCheck > 10000) {
    lastCheck = millis();

    // Cek koneksi jaringan GSM
    if (!modem.isNetworkConnected()) {
      SerialMon.println("Network disconnected");

      if (!modem.waitForNetwork(180000L, true)) {
        SerialMon.println(" fail (Network)");
        hardwareResetModem();
        delay(1000);
        return;
      }

      if (modem.isNetworkConnected()) {
        SerialMon.println("Network re-connected");
      }

#if TINY_GSM_USE_GPRS
      // Cek koneksi GPRS
      if (!modem.isGprsConnected()) {
        SerialMon.println("GPRS disconnected!");
        SerialMon.print(F("Connecting to "));
        SerialMon.println(apn);

        if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
          SerialMon.println(" fail (GPRS)");
          hardwareResetModem();
          return;
        }

        if (modem.isGprsConnected()) {
          SerialMon.println("GPRS reconnected");
        }
      }
#endif
    }
  }

  // MQTT tidak terhubung
  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");

    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;

      if (mqttConnect()) {
        SerialMon.println("MQTT connected");
        mqttFailCount = 0;  // Reset counter
        lastReconnectAttempt = 0;
      } else {
        SerialMon.print("MQTT connect failed, state: ");
        SerialMon.println(mqtt.state());
        SerialMon.print("Reason: ");
        SerialMon.println(mqttStateString(mqtt.state()));
        mqttFailCount++;

        if (mqttFailCount >= 5) {
          SerialMon.println("Too many MQTT failures, resetting modem...");
          hardwareResetModem();
          mqttFailCount = 0;
          return;
        }
      }
    }

    delay(1000);
    return;
  }



  // Tambahan opsional: reset sistem setiap 24 jam (failsafe)
  if (millis() > 86400000L) {
    SerialMon.println("Restarting device after 24 hours...");
    ESP.restart();  // Jika pakai ESP
    // Atau gunakan software reset untuk platform lain
  }
  String timee = modem.getGSMDateTime(DATE_FULL);
  //  SerialMon.print("Current Datetime: ");
  //  SerialMon.println(time);

  int jam = extractHourFromGSMTime(timee);
  if (jam >= 9 && jam < 17) {
    pompa(255); // Hidupkan pompa PWM 255
  } else {
    pompa(0);   // Matikan pompa
  }

  SensorData sensorData = readSensorXYMD02();
  SensorDataINA219 sensorDataINA219 = readSensorINA219();
  SensorDataDS18B20 sensorDataDS18B20 = readSensorDS18B20();
  SensorDataFlow sensorDataFlow = readSensorFlow();

  unsigned long currentMillis = millis();
  if (currentMillis - lastMqttSend >= mqttInterval) {
    lastMqttSend = currentMillis;



    SerialMon.print("Temperature: ");
    SerialMon.print(sensorData.temperature);
    SerialMon.print(" °C\t");
    SerialMon.print("Humidity: ");
    SerialMon.print(sensorData.humidity);
    SerialMon.println(" %RH ");

    SerialMon.print("Solar Voltage: ");
    SerialMon.print(sensorDataINA219.solar_busVoltage);
    SerialMon.print(" V\t");
    SerialMon.print("Solar Current: ");
    SerialMon.print(sensorDataINA219.solar_current_mA);
    SerialMon.print(" mA\t");
    SerialMon.print("Solar Power: ");
    SerialMon.print(sensorDataINA219.solar_power_mW);
    SerialMon.println(" mW\t");

    SerialMon.print("Battery Voltage: ");
    SerialMon.print(sensorDataINA219.battery_busVoltage);
    SerialMon.print(" V\t");
    SerialMon.print("Battery Current: ");
    SerialMon.print(sensorDataINA219.battery_current_mA);
    SerialMon.print(" mA\t");
    SerialMon.print("Battery Power: ");
    SerialMon.print(sensorDataINA219.battery_power_mW);
    SerialMon.println(" mW\t");

    SerialMon.print("Load Voltage: ");
    SerialMon.print(sensorDataINA219.load_busVoltage);
    SerialMon.print(" V\t");
    SerialMon.print("Load Current: ");
    SerialMon.print(sensorDataINA219.load_current_mA);
    SerialMon.print(" mA\t");
    SerialMon.print("Load Power: ");
    SerialMon.print(sensorDataINA219.load_power_mW);
    SerialMon.println(" mW\t");

    SerialMon.print("Water Temperature: ");
    SerialMon.print(sensorDataDS18B20.temperatur_air);
    SerialMon.println(" °C\t");
    SerialMon.print("Water Flow: ");
    SerialMon.print(sensorDataFlow.flow_air);
    SerialMon.println(" L/min\t");

    sendToMQTT(timee,
               sensorData.temperature,
               sensorData.humidity,
               sensorDataINA219.solar_busVoltage,
               sensorDataINA219.solar_current_mA,
               sensorDataINA219.solar_power_mW,
               sensorDataINA219.battery_busVoltage,
               sensorDataINA219.battery_current_mA,
               sensorDataINA219.battery_power_mW,
               sensorDataINA219.load_busVoltage,
               sensorDataINA219.load_current_mA,
               sensorDataINA219.load_power_mW,
               sensorDataDS18B20.temperatur_air,
               sensorDataFlow.flow_air);
  }


  mqtt.loop();
}
