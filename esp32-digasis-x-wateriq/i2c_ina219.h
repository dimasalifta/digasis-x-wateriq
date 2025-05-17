#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 load_ina219(0x44);
Adafruit_INA219 battery_ina219(0x40);
Adafruit_INA219 solar_ina219(0x41);

void i2c_ina219_setup() {
  SerialMon.print("i2c_load_ina219_setup....");
  int retry = 0;
  while (!load_ina219.begin() && retry < 10) {
    SerialMon.print(".");
    retry++;
    delay(500);
  }
  if (retry >= 10) {
    SerialMon.println("\nGagal menemukan load INA219 setelah 10 percobaan!");
  } else {
    SerialMon.println("Success!");
  }

  SerialMon.print("i2c_battery_ina219_setup....");
  retry = 0;
  while (!battery_ina219.begin() && retry < 10) {
    SerialMon.print(".");
    retry++;
    delay(500);
  }
  if (retry >= 10) {
    SerialMon.println("\nGagal menemukan battery INA219 setelah 10 percobaan!");
  } else {
    SerialMon.println("Success!");
  }

  SerialMon.print("i2c_solar_ina219_setup....");
  retry = 0;
  while (!solar_ina219.begin() && retry < 10) {
    SerialMon.print(".");
    retry++;
    delay(500);
  }
  if (retry >= 10) {
    SerialMon.println("\nGagal menemukan solar INA219 setelah 10 percobaan!");
  } else {
    SerialMon.println("Success!");
  }
}


struct SensorDataINA219 {
  float load_busVoltage;
  float load_current_mA;
  float load_power_mW;
  float solar_busVoltage;
  float solar_current_mA;
  float solar_power_mW;
  float battery_busVoltage;
  float battery_current_mA;
  float battery_power_mW;
};

SensorDataINA219 readSensorINA219() {
  SensorDataINA219 data = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  data.load_busVoltage = load_ina219.getBusVoltage_V();
  data.load_current_mA = load_ina219.getCurrent_mA();
  data.load_power_mW = load_ina219.getPower_mW();

  data.battery_busVoltage = battery_ina219.getBusVoltage_V();
  data.battery_current_mA = battery_ina219.getCurrent_mA();
  data.battery_power_mW = battery_ina219.getPower_mW();

  data.solar_busVoltage = solar_ina219.getBusVoltage_V();
  data.solar_current_mA = solar_ina219.getCurrent_mA();
  data.solar_power_mW = solar_ina219.getPower_mW();

  return data;
}
