#include "SGP30.h"
#include "DFRobot_BME680_I2C.h"
/************************************************************************************
//    FILE: SGP30_demo_async.ino
//  AUTHOR: Rob Tillaart
//    DATE: 2021-06-24
//     URL: https://github.com/RobTillaart/SGP30
//          https://www.adafruit.com/product/3709
/************************************************************************************/
/************************************************************************************
//    FILE: DFRobot_BME680_I2C.ino
//  AUTHOR: Jie Han
//    DATE: 2017-12-07
//     URL: https://github.com/DFRobot/DFRobot_BME680
//          http://www.dfrobot.com
/************************************************************************************/
class TVOC {

private:
  #define pinA A0  //analog sensor
  SGP30 sgp;
  DFRobot_BME680_I2C *bme;
  unsigned long lastRead; 
  unsigned long lastUpdate; 
  int averagePos;
  static const int sampleLength = 20;
  int averageVals[sampleLength];
  int tvocLast;
  float temperatureLast;
  int humidityLast;
  int pressureLast;
  
  void qualityCalculation() {
    //10: 50-450; 10=30: 145=1000(145-45=100) 150=1500 157.5=2000 160=2500 165=3000(165-45=120) 170=4000 175=5000 180=6000
    //30: 50-6000;
    //68: 850-0   ; 68=30: 45=1000(550-450=100) 40=2000 35=3000(550-350=200) //30=4000 25=5000 20=6000
    if ((sensor == 10 and tvoc > 200) or (sensor == 30 and tvoc > 3000) or (sensor == 68 and tvoc > 200)) { 
      quality = 3;
    } else if ((sensor == 10 and tvoc > 100) or (sensor == 30 and tvoc > 1000) or (sensor == 68 and tvoc > 100)) {
      quality = 2;
    } else {
      quality = 1;
    } 
  }

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
  String LibraryVersion;
  bool connected;
  int interval; //read interval in seconds
  int quality; //quality of air
  int tvoc;
  float temperature;
  int humidity;
  int pressure;
  float fixTemperature;
  int fixHumidity;
  int altitude;  
  int sensor;
  bool changed;

  TVOC(int timeInterval = 1) {
    interval = timeInterval;
    bme = new DFRobot_BME680_I2C(0x77);  //0x77 I2C address
  }

 void begin(int sensorAnalog) {
    sensor = sensorAnalog;
    connected = false;
    if (sensor == 10) {
      connected = true;
      ProductName = "MICS5524";      
    } else {
      sgp.begin();
      if (sgp.measureTest()) {
        connected = true;
        sensor = 30;
        ProductName = "SGP30";
        Serial.println(F("SGP30 connected."));
        LibraryVersion = String(SGP30_LIB_VERSION);
        Serial.println("Library: " + LibraryVersion);
        sgp.getFeatureSet();
        sgp.GenericReset(); //can reset all devices, rather run as first
      } else {
        Serial.println(F("SGP30 Not connected."));
      }

      if (bme->begin() == 0) {
        connected = true;
        sensor = 68;
        ProductName = "BME680";
        Serial.println(F("BME680 connected.")); 
      } else {
        Serial.println(F("BME680 Not connected."));
      } 
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
    if (connected == false) {return false;}
    int diff;
    if (sensor == 10) {
      diff = 10;
      bool update = false;
      tvoc = analogRead(pinA) - 50; 
      if (tvoc < 0) tvoc = 1;
      if (tvoc > tvocLast) {
        update = true;
      } else {
        if (millis() - lastUpdate >= 15*1000UL or lastUpdate == 0) {
          lastUpdate = millis();
          update = true;
        }
      }
      if (update) {
        averagePos += 1;
        averageVals[averagePos] = tvoc;   
      }
    } else if (sensor == 30) {
      if (sgp.read()) {
        diff = 50;
        averagePos += 1;
        averageVals[averagePos] = sgp.getTVOC();          
        //if (ignoreCO2 == false) co2 = sgp.getCO2();  no real value
      }
      sgp.request();
    } else if (sensor == 68) {
      diff = 5;
      bme->update();
      tvoc = bme->readGasResistance() / 100;
      //Serial.print("; TVOC:" + String(tvoc)); 
      if (tvoc > 500) {
        if (tvoc > 1000) {
          tvoc = 1;
        } else {
          tvoc = round((1000 - tvoc) / 10);
        }
      } else {
        tvoc = 550 - tvoc;
      }
      averagePos += 1;
      averageVals[averagePos] = tvoc;
      temperature = (round(bme->readTemperature() / 10) / 10) - 2 - fixTemperature;
      humidity = round(bme->readHumidity() / 1000) + fixHumidity;
      if (altitude != 0) {
        pressure = round(bme->readPressure() / 100 / pow(1 - ((0.0065 * altitude) / (temperature + (0.0065 * altitude) + 273.15)), 5.257)); 
      } else {
        pressure = round(bme->readPressure() / 100);
      }
      bme->startConvert();
    } 

    tvoc = avarage(averageVals, sampleLength, false);
    if (averagePos == sampleLength) {averagePos = 0;}

    if (abs(tvoc - tvocLast) > diff or temperature != temperatureLast or humidity != humidityLast or pressure != pressureLast) {
      tvocLast = tvoc;
      temperatureLast = temperature;
      humidityLast = humidity;
      pressureLast = pressure;
      qualityCalculation();
      return true;
    }
    return false; 
  }

  void calibrate(int temperature, int humidity) {
    if (sensor == 30 and temperature != 0 and humidity != 0) sgp.setRelHumidity(temperature, humidity);
  }

};
