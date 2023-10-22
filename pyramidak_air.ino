#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "Memory.h"
#include "cMQTT.h"
#include "cWiFi.h"
#include "TrafficLights.h"
#include "Thermistors.h"
#include "LogicSensor.h"
#include "ParticleMatter.h"
#include "CarbonDioxide.h"
#include "TVOC.h"
#include "AmbientLight.h"
#include "Anemometer.h"
#include "AnalogPercent.h"

SoftwareSerial uradmonitor(D7, D8);
SoftwareSerial airquality(D3, 3);
WiFiServer server(80);
Memory mem;
cMQTT mqtt;
cWiFi wi_fi(&mem);
TrafficLights semafor;
Thermistors termistor1;
Thermistors termistor;
LogicSensor logic;
ParticleMatter pm;
CarbonDioxide co2;
TVOC voc;
AmbientLight light;
Anemometer wind;
AnalogPercent rain;

/// copyright Zdeněk Jantač, all rights reserved. This is NOT public open software.
///////////////////////////////////////////////////////////////////////////////////
/// What is new
/// 1.3.8 float temperature correction
/// 1.3.7 fixed termistor with logic sensor
///////////////////////////////
////    Settings block    /////
///////////////////////////////
String deviceName      = "air";
String firmwareVersion = "1.3.8";
  // Update settings //
String OTA_password    = "";
String update_server   = "192.168.0.100";
  // DEVICE settings //
int device        = 1; //1 = air quality, 2 = smoggie-pm
int sensorAnalog  = 0; //0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC TOGGLE, 7-anemometer, 8-lightmeter TEMT6000, 9-raindrops, 10-TVOC MICS5524
int sensorDigital1= 0; //0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC switch TOGGLE, thermometer 7-DHT11, 8-DHT22, 9-DS18B20
int qualityFinal;      //final air quality
String qualityWord;    //final air quality in word
String newMQTTpass, newWIFIpass, newOTApass;
  // PINs declaration //
#define pinA  A0 //analog sensor
#define pinAP D0 //WiFi AP switch
////////////////////////////////
//// Report to serial port  ////
////////////////////////////////
void report(String msg, bool offset = false) {
  if(offset) {Serial.println("");}
  Serial.println(msg);
}
///////////////////////////////
////      Setup block     /////
///////////////////////////////
void setup() {
  Serial.begin(115200); //9600, 115200
  delay(200);
  eeprom_begin();
  pinMode(pinAP, INPUT);  // Wifi AP
  pinMode(pinA, INPUT);   // Analog Sensor  
  semafor.begin();
  mqtt.begin(deviceName, firmwareVersion);

  voc.begin(sensorAnalog);
  co2.begin();
  pm.begin();
  if (device == 1) pm.begin(&airquality);
  if (device == 2) pm.begin(&uradmonitor);
  termistor.begin();
  termistor1.begin(sensorDigital1);
  light.begin(sensorAnalog);
  wind.begin(sensorAnalog);
  rain.begin(sensorAnalog);

  if (device == 1) wi_fi.LEDextra = 2;
  if (device == 2) wi_fi.LEDextra = 0;
  if (mem.read(3) == 0 or wi_fi.ssid == "" or wi_fi.password == "" or (wi_fi.switchAP == 1 and digitalRead(pinAP) == HIGH)) {
    wi_fi.beginAP(); 
  } else {    
    wi_fi.begin(deviceName);
  }
  update_Arduino_begin();
}
///////////////////////////////
////       Loop block     /////
///////////////////////////////
void loop() {
  
  if (wi_fi.connected() == true) {
    if (wi_fi.connectedJobs() == true) {
      server.begin();
      pm.start();    
    }
    if (wi_fi.APmode == false) {
      mqtt.loop(); 
      if (mqtt.callUpdate == true) update_Server_check();
      update_Arduino_handle(); 
    }
    web_server();
  }
  delay(10); //MUST delay to allow ESP8266 WIFI functions to run
  pm.detect();

  //logic analog sensor
  if (logic.readAnalog(sensorAnalog) == true or mqtt.reportSensors) mqtt.analog(logic.getState(logic.analog));
  //logic digital1 sensor
  if (logic.readDigital(sensorDigital1) == true or mqtt.reportSensors) mqtt.digital(logic.getState(logic.digital));
  //wind analog sensor
  wind.loop();
  if (wind.changed or (wind.connected and mqtt.reportSensors)) mqtt.analog(wind.speed);
  //Raindrops analog sensor
  rain.loop();
  if (rain.changed or (rain.connected and mqtt.reportSensors)) mqtt.analog(rain.percent);
  //light analog sensor
  light.loop();
  if (light.changed or mqtt.reportSensors) {
    mqtt.light(light.lux);
    if (light.connected and device == 1) semafor.brightness(round(float(semafor.diodMax) / 1000 * light.lux));
    //report("diodMax: " + String(semafor.diodMax) + "; lux: " + String(light.lux) + "; calculation: " + String(float(semafor.diodMax) / 1000 * light.lux));
  }
  //mqtt brightness command
  if (mqtt.diodBrightChange > 0) {
    semafor.brightness(mqtt.diodBrightChange);
    mqtt.diodBrightChange = 0;
  }

  int quality = 0;
  //Particle Matter sensor
  pm.loop();
  if (pm.changed or mqtt.reportSensors) mqtt.pm(pm.values);
  if (pm.quality > quality) quality = pm.quality;
  //Carbon Dioxide sensor
  co2.loop();
  if (co2.changed or mqtt.reportSensors) mqtt.co2(co2.co2);
  if (co2.quality > quality) quality = co2.quality;
  //Volatile Organic Compound sensor
  voc.loop();
  if (voc.changed or mqtt.reportSensors) mqtt.voc(voc.tvoc);  
  if (voc.quality > quality) quality = voc.quality;
  //Termistors
  termistor1.loop();
  termistor.loop();
  if (termistor.changed or termistor1.changed or co2.changed or (voc.changed and voc.sensor == 68) or mqtt.reportTemp or mqtt.reportSensors) {
    mqtt.reportTemp = false;
    float temperature = 0;
    int humidity = 0;
    int pressure = 0;
    if (termistor.connected) {
      temperature = termistor.temperature;
      humidity = termistor.humidity;
      pressure = termistor.pressure;
    }
    if (termistor1.connected) {
      if (termistor1.temperature < temperature or termistor.connected == false) temperature = termistor1.temperature;
      if (termistor1.humidity > humidity) humidity = termistor1.humidity;
    }
    if (co2.connected) {
      if (co2.temperature < temperature or (termistor.connected == false and termistor1.connected == false)) temperature = co2.temperature;
      if (co2.humidity > humidity) humidity = co2.humidity;
    }
    if (voc.sensor == 68) {
      if (voc.temperature < temperature or (termistor.connected == false and termistor1.connected == false and co2.connected == false)) temperature = voc.temperature;
      if (voc.humidity > humidity) humidity = voc.humidity;
      if (termistor.connected == false) pressure = voc.pressure;
    }
    if (mqtt.temperature != -100 and mqtt.temperature < temperature) temperature = mqtt.temperature;
    mqtt.temp(temperature, humidity, pressure);
    termistor.qualityCalculation(temperature, humidity);    
    voc.calibrate(temperature, humidity);
  }
  if (termistor.quality > quality) quality = termistor.quality; 
  //Final quality report
  if (quality != 0 and quality != qualityFinal or mqtt.reportSensors) {
    qualityFinal = quality;
    if (qualityFinal == 1) qualityWord = "Good";
    if (qualityFinal == 2) qualityWord = "Fair";
    if (qualityFinal == 3) qualityWord = "Bad";
    if (device == 1) semafor.light(qualityFinal);
    mqtt.quality(qualityFinal, qualityWord);
  }
  mqtt.reportSensors = false;
}



