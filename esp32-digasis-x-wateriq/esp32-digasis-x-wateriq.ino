#include "sim800lv2.h"
#include "mqtt.h"
#include "rs485_xymd02.h"
#include "i2c_ina219.h"

void setup() {
  SerialMon.begin(115200);
  delay(2000);
  sim800lv2_setup();
  mqtt_setup();
  rs485_xymd02_setup();
  i2c_ina219_setup();
  SerialMon.println("DEVICE READY!!!");
}

void loop() {
  // Make sure we're still registered on the network
  if (!modem.isNetworkConnected()) {
    SerialMon.println("Network disconnected");
    if (!modem.waitForNetwork(180000L, true)) {
      SerialMon.println(" fail");
      delay(1000);
      return;
    }
    if (modem.isNetworkConnected()) {
      SerialMon.println("Network re-connected");
    }

#if TINY_GSM_USE_GPRS
    // and make sure GPRS/EPS is still connected
    if (!modem.isGprsConnected()) {
      sendATCommand("AT+SAPBR=1,1", "OK", 5000);
      SerialMon.println("GPRS disconnected!");
      SerialMon.print(F("Connecting to "));
      SerialMon.print(apn);
      if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.print(" fail");
        delay(10000);
        return;
      }
      if (modem.isGprsConnected()) {
        SerialMon.print(" GPRS reconnected");
      }
    }
#endif
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
  SensorData sensorData = readSensorXYMD02();
  SensorDataINA219 sensorDataINA219 = readSensorINA219();

  unsigned long currentMillis = millis();
  if (currentMillis - lastMqttSend >= mqttInterval) {
    lastMqttSend = currentMillis;



    SerialMon.print("Temperature: ");
    SerialMon.print(sensorData.temperature);
    SerialMon.print(" °C\t");
    SerialMon.print("Humidity: ");
    SerialMon.print(sensorData.humidity);
    SerialMon.print(" %RH ");
    SerialMon.print("Battery Voltage: ");
    SerialMon.print(sensorDataINA219.battery_busVoltage);
    SerialMon.print(" V\t");
    SerialMon.print("Battery Current: ");
    SerialMon.print(sensorDataINA219.battery_current_mA);
    SerialMon.print(" mA\t");
    SerialMon.print("Battery Power: ");
    SerialMon.print(sensorDataINA219.battery_power_mW);
    SerialMon.print(" mW\t");

    sendToMQTT(sensorData.temperature, sensorData.humidity, sensorDataINA219.load_busVoltage, sensorDataINA219.load_current_mA, sensorDataINA219.load_power_mW, sensorDataINA219.battery_busVoltage, sensorDataINA219.battery_current_mA, sensorDataINA219.battery_power_mW, sensorDataINA219.solar_busVoltage, sensorDataINA219.solar_current_mA, sensorDataINA219.solar_power_mW);
  }


  mqtt.loop();
}
