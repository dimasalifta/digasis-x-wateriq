#include <Arduino.h>
#include <ModbusMaster.h>
#include <HardwareSerial.h>
#define RX_PIN 4
#define TX_PIN 0

HardwareSerial MAX485(1);  // Gunakan UART1

ModbusMaster XYMD02;

unsigned long previousMicros = 0;  // Waktu terakhir sensor dibaca
const unsigned long interval = 1500000;  // Interval pembacaan dalam mikrodetik (1.5 detik)
const unsigned long delay_transmission = 5000;

void rs485_xymd02_setup() {
//  Serial.begin(115200);
  Serial.print("rs485_xymd02_setup....");
  MAX485.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // Baud rate sesuai sensor
  XYMD02.begin(1, MAX485); // Slave ID sensor (default 1)
  Serial.println("Success!");
}

struct SensorData {
  float temperature;
  float humidity;
};

SensorData readSensorXYMD02() {
  SensorData data = {0.0, 0.0};
  uint8_t result = XYMD02.readInputRegisters(0x0000, 2);
  if (result == XYMD02.ku8MBSuccess) {
    data.temperature = XYMD02.getResponseBuffer(0) / 10.0;
    data.humidity = XYMD02.getResponseBuffer(1) / 10.0;
  } else {
    Serial.print("Sensor XYMD02 Read Error: 0x");
    Serial.println(result, HEX);
  }
  return data;
}
