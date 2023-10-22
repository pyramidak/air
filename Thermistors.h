#include <DHTesp.h> //senzor teploty DHT22
#include <OneWire.h> //wire senzoru DS18B20
#include <DallasTemperature.h> //senzor teploty DS18B20
//#include <Adafruit_SHT4x.h>
#include <SensirionI2CSht4x.h>
#include <BMx280I2C.h>
#include <Wire.h>
#include <Arduino.h>

class Thermistors {

private:
  const byte pinD1 = D5;
  DHTesp dht;
  DallasTemperature *dsb;
  int sensor;
  float temperatureLast;
  int humidityLast;
  int pressureLast;
  unsigned long lastRead; 
  OneWire wire1; //gpio
  //Adafruit_SHT4x sht4 = Adafruit_SHT4x();
  SensirionI2CSht4x sht4x;
  BMx280I2C *bmx280;

public:
  float fixTemperature;
  int fixHumidity;
  int altitude;
  float temperature;
  int humidity;
  int pressure;
  String ProductName;
  String SerialNumber;
  int quality; //quality of air
  bool connected;
  int interval; //read interval in seconds
  bool changed;
  int temperatureIndex2 = 27;
  int temperatureIndex3 = 30;
  int humidityIndex2 = 60;
  int humidityIndex3 = 80;

  Thermistors(int timeInterval = 2) {
    interval = timeInterval;
    bmx280 = new BMx280I2C(0x76);
  }

  void begin(int sensorDigital = -1) {
    sensor = sensorDigital;
    connected = false;
    if (sensor == 7) {
      wire1.begin(pinD1);
      dht.setup(pinD1, DHTesp::DHT11);
      ProductName = "DHT11";
      connected = true;
    } else if (sensor == 8) {
      wire1.begin(pinD1);
      dht.setup(pinD1, DHTesp::DHT22);
      ProductName = "DHT22";
      connected = true;
    } else if (sensor == 9) {
      wire1.begin(pinD1);
      dsb = new DallasTemperature(&wire1);
      dsb->begin();
      ProductName = "DS18B20";
      connected = true;
    } else if (sensor >= 0) {      
    } else {
      // SENSIRION
      Wire.begin();
      sht4x.begin(Wire);
      uint16_t error;
      uint32_t serialNumber;
      error = sht4x.serialNumber(serialNumber);
      if (error) {
        Serial.println("SHT40 Not connected.");
      } else {
        connected = true;
        sensor = 40;
        Serial.println("SHT40 connected.");
        ProductName = "SHT40";
        SerialNumber = String(serialNumber);
      }
      if (!bmx280->begin()) {
        Serial.println("BMx280 Not connected.");
      } else {
        connected = true;
        sensor = 28;
        if (bmx280->isBME280()) {
          Serial.println("BME280 connected.");
          ProductName = "BME280";
        } else {
          Serial.println("BMP280 connected.");
          ProductName = "BMP280";
        }
        // Copyright (c) 2018 Gregor Christandl
        bmx280->resetToDefaults();
        bmx280->writeOversamplingPressure(BMx280MI::OSRS_P_x16);
	      bmx280->writeOversamplingTemperature(BMx280MI::OSRS_T_x16);
        if (bmx280->isBME280()) bmx280->writeOversamplingHumidity(BMx280MI::OSRS_H_x16);
        //set the sensor to "normal" mode with 4 measurement per second:
        //bmx280.writeStandbyTime(BMx280MI::T_SB_3);
        //bmx280.writePowerMode(BMx280MI::BMx280_MODE_NORMAL);
      }
      /* ADAFRUIT
      if (sht4.begin()) {
         connected = true;
         ProductName = "SHT40";
         SerialNumber = String(sht4.readSerial());
         sht4.setPrecision(SHT4X_HIGH_PRECISION); 
         // You can have 3 different precisions, higher precision takes longer read
         //SHT4X_MED_PRECISION
         //SHT4X_LOW_PRECISION
         sht4.setHeater(SHT4X_NO_HEATER);
         // Higher heat and longer times uses more power and reads will take longer too!
         //SHT4X_HIGH_HEATER_1S SHT4X_HIGH_HEATER_100MS
         //SHT4X_MED_HEATER_1S SHT4X_MED_HEATER_100MS
         //SHT4X_LOW_HEATER_1S SHT4X_LOW_HEATER_100MS
      }
      */
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
    float temp = 0;
    float humid = 0;
    float press = 0;
    if (sensor == 7 or sensor == 8) {
      temp = dht.getTemperature();  
      humid = dht.getHumidity();
    } else if (sensor == 9) {
      dsb->requestTemperatures();
      temp = dsb->getTempCByIndex(0);
    } else if (sensor == 28) {
      if (bmx280->hasValue()) {
        temp = bmx280->getTemperature();
        press = bmx280->getPressure();
        if (bmx280->isBME280()) humid = bmx280->getHumidity();
      }
      bmx280->measure();
    } else if (sensor == 40) {
      // SENSIRION
      uint16_t error;
      error = sht4x.measureHighPrecision(temp, humid);
      if (error) return false;
      /* ADAFRUIT
      sensors_event_t humid0, temp0;
      sht4.getEvent(&humid0, &temp0);
      temp = temp0.temperature;
      humid = humid0.relative_humidity;
      */
    }  
    if (temp > -99 and temp < 99) {
      temperature = (round(temp * 10)/10) - fixTemperature;
    }
    if (humid > 0 and humid < 100) {
      humidity = round(humid) + fixHumidity;
    }
    if (press > 10000) {
      if (altitude != 0) {
        pressure = round(press / 100 / pow(1 - ((0.0065 * altitude) / (temperature + (0.0065 * altitude) + 273.15)), 5.257)); 
      } else {
        pressure = round(press / 100);
      }
    }
    if (temperature != temperatureLast or humidity != humidityLast or pressure != pressureLast) {
      temperatureLast = temperature;
      humidityLast = humidity;
      pressureLast = pressure;
      qualityCalculation(temperature, humidity);
      return true;  
    }  
    return false;
  }

  void qualityCalculation(float temp, int humid) {
    if (temp > temperatureIndex3 or humid > humidityIndex3) {
      quality = 3;
    } else if (temp > temperatureIndex2 or humid > humidityIndex2) {
      quality = 2;
    } else {
      quality = 1;
    }
  }

};
