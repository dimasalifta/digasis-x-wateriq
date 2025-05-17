/*
    Application:
    - Interface water flow sensor with ESP32 board.

    Board:
    - ESP32 Dev Module
      https://my.cytron.io/p-node32-lite-wifi-and-bluetooth-development-kit
    Sensor:
    - G 1/2 Water Flow Sensor
      https://my.cytron.io/p-g-1-2-water-flow-sensor
*/
#define SENSOR 26

long currentMillis = 0;
long previousMillis = 0;
int intervall = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void watersetup() {
  // Serial.begin(115200);

  pinMode(SENSOR, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}


struct SensorDataFlow {
  float flow_air;
};


SensorDataFlow readSensorFlow() {
  SensorDataFlow data = { 0.0 };

  currentMillis = millis();
  if (currentMillis - previousMillis > intervall) {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;

    data.flow_air = flowRate;
    return data;
  }

  // fallback jika belum waktunya update
  return data;
}
