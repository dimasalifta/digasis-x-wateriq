#include "sim800lv2.h"
#include "mqtt.h"
#include "rs485_xymd02.h"
#include "i2c_ina219.h"

void setup() {
  Serial.begin(115200);
  sim800lv2_setup();
  mqtt_setup();
  rs485_xymd02_setup();
  i2c_ina219_setup();
  Serial.println("DEVICE READY!!!");
}

void loop() {
  // Make sure we're still registered on the network
  if (!modem.isNetworkConnected()) {
    Serial.println("Network disconnected");
    if (!modem.waitForNetwork(180000L, true)) {
      Serial.println(" fail");
      delay(10000);
      return;
    }
    if (modem.isNetworkConnected()) {
      Serial.println("Network re-connected");
    }

#if TINY_GSM_USE_GPRS
    // and make sure GPRS/EPS is still connected
    if (!modem.isGprsConnected()) {
      Serial.println("GPRS disconnected!");
      Serial.print(F("Connecting to "));
      Serial.print(apn);
      if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        Serial.println(" fail");
        delay(10000);
        return;
      }
      if (modem.isGprsConnected()) {
        Serial.println("GPRS reconnected");
      }
    }
#endif
  }

  if (!mqtt.connected()) {
    Serial.println("=== MQTT NOT CONNECTED ===");
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



    Serial.print("Temperature: ");
    Serial.print(sensorData.temperature);
    Serial.print(" °C\t");
    Serial.print("Humidity: ");
    Serial.print(sensorData.humidity);
    Serial.print(" %RH ");
    Serial.print("Voltage: ");
    Serial.print(sensorDataINA219.busVoltage);
    Serial.print(" V\t");
    Serial.print("Current: ");
    Serial.print(sensorDataINA219.current_mA);
    Serial.print(" mA ");
    Serial.print("Power: ");
    Serial.print(sensorDataINA219.power_mW);
    Serial.print(" mW ");

    sendToMQTT(sensorData.temperature, sensorData.humidity, sensorDataINA219.busVoltage, sensorDataINA219.current_mA, sensorDataINA219.power_mW);
  }


  mqtt.loop();
}
