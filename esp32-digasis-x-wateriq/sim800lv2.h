#define TINY_GSM_MODEM_SIM800

//#define Serial Serial
//#define TINY_GSM_DEBUG Serial

#define RXD2 16
#define TXD2 17
//#define SIM800_RST 9  // Pin Reset SIM800L

#include <TinyGsmClient.h>
HardwareSerial SerialAT(2); // RX, TX
//// Range to attempt to autobaud
//#define GSM_AUTOBAUD_MIN 9600
//#define GSM_AUTOBAUD_MAX 115200

// Your GPRS credentials, if any
const char apn[] = "by.u";
const char gprsUser[] = "";
const char gprsPass[] = "";

int failedChecks = 0;  // Hitung kegagalan koneksi SIM800L
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

// Fungsi untuk mengirim perintah AT ke SIM800L
bool sendATCommand(String command, String expected, unsigned int timeout) {
  Serial.print("SEND: ");
  Serial.println(command);

  SerialAT.println(command);
  unsigned long startTime = millis();
  String response = "";

  while (millis() - startTime < timeout) {
    while (SerialAT.available()) {
      char c = SerialAT.read();
      response += c;
    }
    if (response.indexOf(expected) != -1) {
      Serial.print("RESP: ");
      Serial.println(response);
      return true;
    }
  }

  Serial.println("FAILED: " + command);
  return false;
}

void sim800lv2_setup() {
//  // Set console baud rate
//  Serial.begin(115200);
//  delay(10);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  Serial.println("Sim800lv2 setup Wait...");

  // Set GSM module baud rate
  SerialAT.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();
  //  modem.init();

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem Info: ");
  Serial.println(modemInfo);

  String name = modem.getModemName();
  Serial.print("Modem Name: ");
  Serial.println(name);

  String manufacturer = modem.getModemManufacturer();
  Serial.print("Modem Manufacturer: ");
  Serial.println(manufacturer);

  String hw_ver = modem.getModemModel();
  Serial.print("Modem Hardware Version: ");
  Serial.println(hw_ver);

  String fv_ver = modem.getModemRevision();
  Serial.print("Modem Firmware Version: ");
  Serial.println(fv_ver);

  // Unlock your SIM card with a PIN if needed
  //  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
  //    modem.simUnlock(GSM_PIN);
  //  }

  // The XBee must run the gprsConnect function BEFORE waiting for network!
  modem.gprsConnect(apn, gprsUser, gprsPass);

  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isNetworkConnected()) {
    Serial.println("Network connected");
  }


  // GPRS connection parameters are usually set after network registration
  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isGprsConnected()) {
    Serial.println("GPRS connected");

    String ccid = modem.getSimCCID();
    Serial.print("Modem CCID: ");
    Serial.println(ccid);

    String imei = modem.getIMEI();
    Serial.print("Modem IMEI: ");
    Serial.println(imei);

    String imsi = modem.getIMSI();
    Serial.print("Modem IMSI: ");
    Serial.println(imsi);

    String cop = modem.getOperator();
    Serial.print("Modem Operator: ");
    Serial.println(cop);

    IPAddress local = modem.localIP();
    Serial.print("Modem LocalIP: ");
    Serial.println(local);

    int csq = modem.getSignalQuality();
    Serial.print("Modem Signal Quality: ");
    Serial.println(csq);


    String time = modem.getGSMDateTime(DATE_FULL);
    Serial.print("Current Datetime: ");
    Serial.println(time);

  }

  sendATCommand("AT", "OK", 2000);
  sendATCommand("AT+CSCLK=0", "OK", 2000);  // Disable Sleep Mode
  //  sendATCommand("AT+CFUN=1", "OK", 2000);   // Aktifkan full functionality
  //  sendATCommand("AT&F", "OK", 2000);
  sendATCommand("AT&W", "OK", 2000);   // Save
  delay(1000);
}
