#include "sim800lv2.h"
#include "mqtt.h"
#include "rs485_xymd02.h"
#include "i2c_ina219.h"
#include "ds18b20.h"

#define MODEM_RST 15
void hardwareResetModem() {
  SerialMon.println("== HARDWARE RESET MODEM ==");
  digitalWrite(MODEM_RST, LOW);
  delay(1000); // tahan 1 detik
  digitalWrite(MODEM_RST, HIGH);
  delay(5000); // tunggu modem bangun
}


void setup() {
  SerialMon.begin(115200);
  delay(2000);
  sim800lv2_setup();
  mqtt_setup();
  rs485_xymd02_setup();
  i2c_ina219_setup();
  ds18b20_setup();
  pinMode(MODEM_RST, OUTPUT);
  digitalWrite(MODEM_RST, HIGH); // pastikan awalnya tidak di-reset
  SerialMon.println("DEVICE READY!!!");
}

void loop() {
  // Make sure we're still registered on the network
  if (!modem.isNetworkConnected()) {
    SerialMon.println("Network disconnected");
    if (!modem.waitForNetwork(180000L, true)) {
      SerialMon.println(" fail (Network)");
      sendATCommand("AT+CFUN=1,1", "OK", 5000);
      // Reset SIM800L
      //      digitalWrite(SIM800_RST, LOW);
      //      delay(200);  // minimal 100 ms
      //      digitalWrite(SIM800_RST, HIGH);
      delay(1000);
      return;
    }
    if (modem.isNetworkConnected()) {
      SerialMon.println("Network re-connected");
    }

#if TINY_GSM_USE_GPRS
    // and make sure GPRS/EPS is still connected
    if (!modem.isGprsConnected()) {
      SerialMon.println("GPRS disconnected!");
      SerialMon.print(F("Connecting to "));
      SerialMon.print(apn);
      if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.print(" fail (GPRS)");
        sendATCommand("AT+CFUN=1,1", "OK", 5000);
        // Reset SIM800L
        digitalWrite(SIM800_RST, LOW);
        delay(200);  // minimal 100 ms
        digitalWrite(SIM800_RST, HIGH);
        delay(1000);
        return;
      }
      if (modem.isGprsConnected()) {
        SerialMon.print(" GPRS reconnected");
      }
    }
#endif
  }

  if (!modem.waitForNetwork(180000L, true)) {
    SerialMon.println(" fail (Network)");
    sendATCommand("AT+CFUN=1,1", "OK", 5000);
    delay(1000);

    // Tambahan:
    hardwareResetModem();

    return;
  }

  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }
    delay(1000);
    return;
  }
  String timee = modem.getGSMDateTime(DATE_FULL);
  //  SerialMon.print("Current Datetime: ");
  //  SerialMon.println(time);
  //  SensorData sensorData = readSensorXYMD02();
  SensorDataINA219 sensorDataINA219 = readSensorINA219();
  SensorDataDS18B20 sensorDataDS18B20 = readSensorDS18B20();

  unsigned long currentMillis = millis();
  if (currentMillis - lastMqttSend >= mqttInterval) {
    lastMqttSend = currentMillis;



    //    SerialMon.print("Temperature: ");
    //    SerialMon.print(sensorData.temperature);
    //    SerialMon.print(" °C\t");
    //    SerialMon.print("Humidity: ");
    //    SerialMon.print(sensorData.humidity);
    //    SerialMon.println(" %RH ");

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
    //    sendToMQTT(sensorData.temperature, sensorData.humidity, sensorDataINA219.load_busVoltage, sensorDataINA219.load_current_mA, sensorDataINA219.load_power_mW, sensorDataINA219.battery_busVoltage, sensorDataINA219.battery_current_mA, sensorDataINA219.battery_power_mW, sensorDataINA219.solar_busVoltage, sensorDataINA219.solar_current_mA, sensorDataINA219.solar_power_mW);
    sendToMQTT(timee, 1, 1, sensorDataINA219.load_busVoltage, sensorDataINA219.load_current_mA, sensorDataINA219.load_power_mW, sensorDataINA219.battery_busVoltage, sensorDataINA219.battery_current_mA, sensorDataINA219.battery_power_mW, sensorDataINA219.solar_busVoltage, sensorDataINA219.solar_current_mA, sensorDataINA219.solar_power_mW, sensorDataDS18B20.temperatur_air);
  }


  mqtt.loop();
}
