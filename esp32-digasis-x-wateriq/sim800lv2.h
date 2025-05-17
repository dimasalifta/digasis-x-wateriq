#define TINY_GSM_MODEM_SIM800

#define SerialMon Serial
#define TINY_GSM_DEBUG SerialMon

#define RXD2 16
#define TXD2 17
//#define SIM800_RST 9  // Pin Reset SIM800L

#include <TinyGsmClient.h>
HardwareSerial SerialAT(2); // RX, TX


// Your GPRS credentials, if any
const char apn[] = "by.u";
const char gprsUser[] = "";
const char gprsPass[] = "";

int failedChecks = 0;  // Hitung kegagalan koneksi SIM800L
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

// Fungsi untuk mengirim perintah AT ke SIM800L
bool sendATCommand(String command, String expected, unsigned int timeout) {
  SerialMon.print("SEND: ");
  SerialMon.println(command);

  SerialAT.println(command);
  unsigned long startTime = millis();
  String response = "";

  while (millis() - startTime < timeout) {
    while (SerialAT.available()) {
      char c = SerialAT.read();
      response += c;
    }
    if (response.indexOf(expected) != -1) {
      SerialMon.print("RESP: ");
      SerialMon.println(response);
      return true;
    }
  }

  SerialMon.println("FAILED: " + command);
  return false;
}

void sim800lv2_setup() {

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  SerialMon.println("Sim800lv2 setup Wait...");

  // Set GSM module baud rate
  SerialAT.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  //  modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

  String name = modem.getModemName();
  SerialMon.print("Modem Name: ");
  SerialMon.println(name);

  String manufacturer = modem.getModemManufacturer();
  SerialMon.print("Modem Manufacturer: ");
  SerialMon.println(manufacturer);

  String hw_ver = modem.getModemModel();
  SerialMon.print("Modem Hardware Version: ");
  SerialMon.println(hw_ver);

  String fv_ver = modem.getModemRevision();
  SerialMon.print("Modem Firmware Version: ");
  SerialMon.println(fv_ver);

  // Unlock your SIM card with a PIN if needed
  //  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
  //    modem.simUnlock(GSM_PIN);
  //  }

  // The XBee must run the gprsConnect function BEFORE waiting for network!
  modem.gprsConnect(apn, gprsUser, gprsPass);

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }


  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");

    String ccid = modem.getSimCCID();
    SerialMon.print("Modem CCID: ");
    SerialMon.println(ccid);

    String imei = modem.getIMEI();
    SerialMon.print("Modem IMEI: ");
    SerialMon.println(imei);

    String imsi = modem.getIMSI();
    SerialMon.print("Modem IMSI: ");
    SerialMon.println(imsi);

    String cop = modem.getOperator();
    SerialMon.print("Modem Operator: ");
    SerialMon.println(cop);

    IPAddress local = modem.localIP();
    SerialMon.print("Modem LocalIP: ");
    SerialMon.println(local);

    int csq = modem.getSignalQuality();
    SerialMon.print("Modem Signal Quality: ");
    SerialMon.println(csq);


    String time = modem.getGSMDateTime(DATE_FULL);
    SerialMon.print("Current Datetime: ");
    SerialMon.println(time);

  }

  sendATCommand("AT", "OK", 2000);
  sendATCommand("AT+CSCLK=0", "OK", 2000);  // Disable Sleep Mode
  //  sendATCommand("AT+CFUN=1", "OK", 2000);   // Aktifkan full functionality
  //  sendATCommand("AT&F", "OK", 2000);
  sendATCommand("AT&W", "OK", 2000);   // Save
  delay(1000);
}
