#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

void i2c_ina219_setup() {
  Serial.print("i2c_ina219_setup....");
  if (!ina219.begin()) {
    Serial.println("Gagal menemukan INA219! Periksa koneksi Anda.");
    while (1);
  }
  Serial.println("Success!");
}


struct SensorDataINA219 {
  float busVoltage;
  float current_mA;
  float power_mW;
};

SensorDataINA219 readSensorINA219() {
  SensorDataINA219 data = {0.0, 0.0, 0.0};

  data.busVoltage = ina219.getBusVoltage_V();
  data.current_mA = ina219.getCurrent_mA();
  data.power_mW = ina219.getPower_mW();
  return data;
}
