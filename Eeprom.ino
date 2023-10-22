//Memory:
//  0: memAlok
//  1: device
//  2: sleep mode
//  3: wifi ready
//  4: WiFi AP switch
//  5: analogSW
//  6: sensorAnalog
//  7: sensorDigital
//  8: sensorDigital2
//  9-21:
// 22: brightness leds
// 23: altitude * 250
// 24: altitude +
// 25: max. wind speed m/s
// 26: min raindrops
// 27: max raindrops
// 28: temperature index 2.
// 29: temperature index 3.
// 30: humidity index 2.
// 31: humidity index 3.
// 32-35:
// 36-50: wifi name
// 51-65: wifi password
// 66-90: mqtt server
// 91-105: mqtt user
//106-120: mqtt password
//121-135: ota password
//136-150: device name
//151-165: server address
//166: temperature correction
//167: humidity correction
//168: remote update off
//169: 
//170-175: web password

void eeprom_begin() { 
  mem.begin();
  device = mem.readAndCheck(device, 1, "devicePurpose", 1, 2);
  report("wifi: " + String(mem.read(3)) );
  wi_fi.switchAP = mem.readAndCheck(wi_fi.switchAP, 4, "wifiSwitch", 0, 2);
  mqtt.updateStart = mem.readAndCheck(1, 168, "updateRemote", 0, 1);
  sensorAnalog = mem.readAndCheck(sensorAnalog, 6, "sensorAnalog", 0, 10);
  sensorDigital1 = mem.readAndCheck(sensorDigital1, 7, "sensorDigital", 0, 9);
  semafor.diodMax = mem.readAndCheck(semafor.diodMax, 22, "brightnessLight", 1, semafor.diodLimit);
  wind.speedMax = mem.readAndCheck(wind.speedMax, 25, "windSpeedMax", 9, 99);
  wi_fi.ssid = mem.readAndCheck(wi_fi.ssid, "wifiSSID", 36, 50, false);
  wi_fi.password = mem.readAndCheck(wi_fi.password, "wifiPass", 51, 65, true);
  if (wi_fi.password != "") newWIFIpass = "*";
  mqtt.server = mem.readAndCheck(mqtt.server, "MQTTserver", 66, 90, false);
  mqtt.user = mem.readAndCheck(mqtt.user, "MQTTuser", 91, 105, false);
  mqtt.password = mem.readAndCheck(mqtt.password, "MQTTpass", 106, 120, true);
  if (mqtt.password != "") newMQTTpass = "*";
  OTA_password = mem.readAndCheck(OTA_password, "OTApass", 121, 135, true);
  if (OTA_password != "") newOTApass = "*";
  deviceName = mem.readAndCheck(deviceName, "deviceName", 136, 150, false);
  update_server = mem.readAndCheck(update_server, "updateServer", 151, 165, false);
  termistor.fixTemperature = mem.readAndCheck(20, 166, "fixTemperature", 0, 99) / 10.0;
  termistor.fixHumidity = mem.readAndCheck(5, 167, "fixHumidity", 0, 9);
  termistor.altitude = mem.readAndCheck(23, 24, "altitude");
  co2.fixTemperature = termistor.fixTemperature;
  co2.fixHumidity = termistor.fixHumidity;
  voc.fixTemperature = termistor.fixTemperature;
  voc.fixHumidity = termistor.fixHumidity;
  voc.altitude = termistor.altitude;
  termistor.temperatureIndex2 = mem.readAndCheck(termistor.temperatureIndex2, 28, "temperatureIndex2", 0, 99);
  termistor.temperatureIndex3 = mem.readAndCheck(termistor.temperatureIndex3, 29, "temperatureIndex3", 0, 99);
  termistor.humidityIndex2 = mem.readAndCheck(termistor.humidityIndex2, 30, "humidityIndex2", 0, 99);
  termistor.humidityIndex3 = mem.readAndCheck(termistor.humidityIndex3, 31, "humidityIndex3", 0, 99);
  rain.min = mem.readAndCheck(rain.min, 26, "minAnalog", 0, 254) * 4.03;
  rain.max = mem.readAndCheck(rain.max, 27, "maxAnalog", 0, 254) * 4.03;
}

