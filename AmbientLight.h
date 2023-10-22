#include <SparkFun_VEML7700_Arduino_Library.h>
#include <Wire.h>
/************************************************************************************
//    FILE: Example1_getLux.ino
//  AUTHOR: Paul Clark
//    DATE: November 4th 2021
//     URL: http://librarymanager/All#SparkFun_VEML7700
//          https://www.sparkfun.com/products/18976
/************************************************************************************/
class AmbientLight {

private:
  #define pinA A0  //analog sensor
  VEML7700 light;
  unsigned long lastRead;
  int luxLast;
  int sensor;

  int avarage(int sample[], int length, bool zero) {
    long together = 0;
    int count = 0;
    for (int i = 0; i < length; i++) {
      if (sample[i] != 0 or zero == true) {
        count += 1;
        together += sample[i];  
      }
    }
    if (count == 0) {
      return 0;
    } else {
      return round(together / count);
    }
  }

public:
  String ProductName;
  bool connected;
  int lux;
  bool changed;

  AmbientLight() {}

  void begin(int sensorAnalog) {
    sensor = sensorAnalog;
    connected = false;
    if (sensor == 8) {
      connected = true;
      ProductName = "TEMT6000";   
    } else {
      Wire.begin();
      if (light.begin() == true) { 
        connected = true;
        ProductName = "VEML7700";
      }
    }
  }

  void loop() {
    changed = false;
    if (connected == true) {
      if (millis() - lastRead >= 500UL or lastRead == 0) {
        lastRead = millis();
        changed = measure();
      }
    }    
  }

  bool measure() {
    if (connected == true) {
      if (sensor == 8) {
        int sample = 10;
        int averageVals[sample];
        for (int i = 0; i < sample; i++) {
          averageVals[i] = analogRead(pinA);
        }
        int reading = avarage(averageVals, sample, false);
        lux = round(reading * 3.3 / 1024 * (1000 / 3.3));
      } else {
        lux = round(light.getLux());
      } 
      if (abs(lux - luxLast) > 20 or (lux < 20 and lux < luxLast)) {
        luxLast = lux;
        return true;
      }
    }
    return false;
  }

};
