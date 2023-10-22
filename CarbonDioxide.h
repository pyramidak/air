#include "paulvha_SCD30.h"
 /************************************************************************************
//  AUTHOR: Nathan Seidle
//    DATE: May 22nd
// PURPOSE: Reading CO2, humidity and temperature from the SCD30
//  AUTHOR: paulvha
//    DATE: august 2018 - january 2019
//   ADDED: support ESP8266 and ESP32
//     URL: https://github.com/paulvha/scd30_on_raspberry
/************************************************************************************/
class CarbonDioxide {

private:
  #define SCD30WIRE Wire
  #define DEBUG 0
  /* define driver debug
 * 0 : no messages
 * 1 : request sending and receiving
 * 2 : request sending and receiving + show protocol errors */
  SCD30 scd;
  unsigned long lastRead;  
  
  void qualityCalculation() {
    if (co2 > 2000) {
      quality = 3;
    } else if (co2 > 1000) {
      quality = 2;
    } else {
      quality = 1;
    } 
  }

  void GetDeviceInfo() {
    uint8_t val[2];
    char buf[(SCD30_SERIAL_NUM_WORDS * 2) +1];

    // Read SCD30 serial number as printed on the device
    // buffer MUST be at least 33 digits (32 serial + 0x0)
    if (scd.getSerialNumber(buf)) {
      Serial.print(F("Serial number: "));
      Serial.println(buf);
      SerialNumber = String(buf);
    } else {
      Serial.println("Could not obtain serial number");
    }

    // read Firmware level
    if (scd.getFirmwareLevel(val) ) {
      Serial.print("Firmware level: Major: ");
      Serial.print(val[0]);
      Serial.print("\t, Minor: ");
      Serial.println(val[1]);
      FirmwareVersion = String(val[0]) + "." + String(val[1]);
    } else {
      Serial.println("Could not obtain firmware level");
    }
  }

public:
  String SerialNumber;
  String ProductName = "SCD30";
  String FirmwareVersion;
  bool connected;
  int interval; //read interval in seconds
  int quality; //quality of air
  int co2, humidity;
  float temperature;
  float fixTemperature;
  int fixHumidity; //corrections
  bool changed;

  CarbonDioxide(int timeInterval = 5) {
    interval = timeInterval;
  }

  void begin() {
    SCD30WIRE.begin();
    // set driver debug level
    scd.setDebug(DEBUG);
    // Begin communication channel;
    //This will cause readings to occur every two seconds
    Serial.print("\r\n");
    if (! scd.begin(SCD30WIRE)) {
      Serial.println(F("SCD30 Not connected."));
      connected = false;
    } else {
      Serial.println(F("SCD30 connected."));
      connected = true;
      // read device info
      GetDeviceInfo();
    }
  }

  void loop() {
    changed = false;
    if (connected == true) {
      if (millis() - lastRead >= interval*1000UL or lastRead == 0) {
        lastRead = millis();
        changed = measure();
      }
    }
  }

  bool measure() {
    if (scd.dataAvailable()) {
      int co2New = scd.getCO2();
      float temperatureNew = round(scd.getTemperature() * 10) / 10 - 2 - fixTemperature;
      int humidityNew = round(scd.getHumidity()) + fixHumidity;
      if (abs(co2New - co2) > 5 or temperatureNew != temperature or humidityNew != humidityNew) {
        co2 = co2New;
        temperature = temperatureNew;
        humidity = humidityNew;
        qualityCalculation();
        return true;
      }
    }
    return false; 
  }

};
