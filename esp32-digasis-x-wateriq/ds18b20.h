#include <OneWire.h>
#include <DallasTemperature.h>

// Pin yang terhubung ke pin DATA DS18B20
#define ONE_WIRE_BUS 23

// Inisialisasi oneWire dan DallasTemperature
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

void ds18b20_setup() {
  ds18b20.begin(); // Mulai komunikasi dengan sensor
}

struct SensorDataDS18B20 {
  float temperatur_air;
};


SensorDataDS18B20 readSensorDS18B20() {
  SensorDataDS18B20 data = {0.0};
  ds18b20.requestTemperatures(); // Minta data suhu dari sensor
  data.temperatur_air = ds18b20.getTempCByIndex(0); // Baca suhu dari sensor pertama (jika lebih dari satu)
  return data;
}
