String menu = "main";
String newFirmwareVersion, lockPass;
bool web_restartNeeded; // restart needed
////////////////////////////////////////////////////////////////////////////////////////////////
////       WEB REQUEST      ////////       WEB REQUEST      ////////       WEB REQUEST      ////
////////////////////////////////////////////////////////////////////////////////////////////////
void web_request(String &header) {
  //MAIN/////////////////////////////////////////////////////////////////////////////////
  if (header.indexOf("GET /main") >= 0) {
    menu = "main";
  } else if (header.indexOf("GET /restart") >= 0) {
    mqtt.disconnect();
    ESP.restart();
  } else if (header.indexOf("GET /reconnect") >= 0) {
    web_restartNeeded = false;
    wi_fi.reconnectNeeded = true;
    mqtt.reconnectNeeded = true;
  } else if (header.indexOf("GET /lock") >= 0) {
    menu = "lock";
  } else if (header.indexOf("GET /device") >= 0) {
    menu = "device";
  } else if (header.indexOf("GET /sensor") >= 0) {
    menu = "sensor";
  } else if (header.indexOf("GET /led") >= 0) {
    menu = "led";
  } else if (header.indexOf("GET /mqtt") >= 0) {
    menu = "mqtt";
  } else if (header.indexOf("GET /wifi") >= 0) {
    menu = "wifi";
  } else if (header.indexOf("GET /clear") >= 0) {
    menu = "clear";
  } else if (header.indexOf("GET /firmware") >= 0) {
    menu = "firmware";     
  //LOCK/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("GET /setlock") >= 0) {
    lockPass = "";
    menu = "main";
  } else if (header.indexOf("?unlock=") >= 0) {     
    lockPass = findSubmit("unlock");
  } else if (header.indexOf("?lock_pass=") >= 0) {
    lockPass = findSubmit("lock_pass");
    mem.write(lockPass, 170, 175);                 
  //DEVICE/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?name=") >= 0) {
    deviceName = setOption(deviceName, "name", "device name", 15, 136, 150, true);
    wi_fi.deviceName = deviceName;
    mqtt.deviceName = deviceName;
  } else if (header.indexOf("?purpose=") >= 0) {
    device = setOption(device, "purpose", "device purpose", 1, 2, 1, true);          
  //SENSOR/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("GET /analog/min") >= 0) {
    mem.write(int(round(rain.reading / 4.03)), 26);
    report("min set: " + String(rain.reading));    
    rain.min = rain.reading;
    rain.measure();
  } else if (header.indexOf("GET /analog/max") >= 0) {
    mem.write(int(round(rain.reading / 4.03)), 27);    
    report("max set: " + String(rain.reading));    
    rain.max = rain.reading;
    rain.measure();    
  } else if (header.indexOf("?1analog=") >= 0) {
    int sensor = sensorAnalog;
    sensorAnalog = setOption(sensorAnalog, "1analog", "analog sensor", 0, 10, 6, false);  
    if (sensor == 10 or sensorAnalog == 10) voc.begin(sensorAnalog);
    light.begin(sensorAnalog);
    wind.begin(sensorAnalog);
    rain.begin(sensorAnalog);
  } else if (header.indexOf("?1digital=") >= 0) {
    sensorDigital1 = setOption(sensorDigital1, "1digital", "digital sensor", 0, 9, 7, false); 
    termistor1.begin(sensorDigital1);
  } else if (header.indexOf("?wind_speed=") >= 0) {
    wind.speedMax = setOption(wind.speedMax, "wind_speed", "wind speed max.", 9, 99, 25, false);
    wind.measure();  
  } else if (header.indexOf("?fix_temp=") >= 0) {
    float value = findSubmit("fix_temp").toFloat();
    if (value < 0) value = abs(value);
    if (value > 9.9) value = 9.9;
    int val = value * 10.0;
    mem.write(val, 166);
    termistor.fixTemperature = value;
    co2.fixTemperature = termistor.fixTemperature;
    voc.fixTemperature = termistor.fixTemperature;
    if (co2.connected) co2.measure();
    if (termistor.connected) termistor.measure();
    if (voc.connected) voc.measure();
  } else if (header.indexOf("?fix_humi=") >= 0) {
    termistor.fixHumidity = setOption(termistor.fixHumidity, "fix_humi", "humidity correction", 0, 25, 167, false);                            
    co2.fixHumidity = termistor.fixHumidity;
    voc.fixHumidity = termistor.fixHumidity;
    if (co2.connected) co2.measure();
    if (termistor.connected) termistor.measure();
    if (voc.connected) voc.measure();
  } else if (header.indexOf("?altitude=") >= 0) {
    termistor.altitude = findSubmit("altitude").toInt(); 
    voc.altitude = termistor.altitude;             
    if (voc.connected) voc.measure();
    if (termistor.connected) termistor.measure();
    mem.write(termistor.altitude, 23, 24);
  } else if (header.indexOf("?index2temp=") >= 0)  {
    termistor.temperatureIndex2 = setOption(termistor.temperatureIndex2, "index2temp", "temperature index 2", 0, 99, 28, false);
    termistor.qualityCalculation(termistor.temperature, termistor.humidity);
  } else if (header.indexOf("?index3temp=") >= 0)  {
    termistor.temperatureIndex3 = setOption(termistor.temperatureIndex3, "index3temp", "temperature index 3", 0, 99, 29, false);
    termistor.qualityCalculation(termistor.temperature, termistor.humidity);    
  } else if (header.indexOf("?index2humid=") >= 0)  {
    termistor.humidityIndex2 = setOption(termistor.humidityIndex2, "index2humid", "humidity index 2", 0, 99, 30, false);
    termistor.qualityCalculation(termistor.temperature, termistor.humidity);
  } else if (header.indexOf("?index3humid=") >= 0)  {
    termistor.humidityIndex3 = setOption(termistor.humidityIndex3, "index3humid", "humidity index 3", 0, 99, 31, false);
    termistor.qualityCalculation(termistor.temperature, termistor.humidity);    
  //LED/////////////////////////////////////////////////////////////////////////////////////       
  } else if (header.indexOf("?diodbright=") >= 0) {
    semafor.diodMax = setOption(semafor.diodMax, "diodbright", "LED brightness", 1, semafor.diodLimit, 22, false);
    semafor.brightness(semafor.diodMax);
  //MQTT/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?server_mqtt=") >= 0) {
    mqtt.server = setOption(mqtt.server, "server_mqtt", "MQTT server", 25, 66, 90, true);
  } else if (header.indexOf("?user_mqtt=") >= 0) {
    mqtt.user = setOption(mqtt.user, "user_mqtt", "MQTT user", 15, 91, 105, true);
  } else if (header.indexOf("?pass_mqtt=") >= 0) {
    newMQTTpass = setOption(newMQTTpass, "pass_mqtt", "MQTT password", 15, 106, 120, true);
    mqtt.password = newMQTTpass;
  //WIFI/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?ssid_wifi=") >= 0) {
    wi_fi.ssid = setOption(wi_fi.ssid, "ssid_wifi", "WIFI SSID", 15, 36, 50, true);
    mem.write(1, 3);
  } else if (header.indexOf("?pass_wifi=") >= 0) {
    newWIFIpass = setOption(newWIFIpass, "pass_wifi", "WIFI password", 15, 51, 65, true);
    wi_fi.password = newWIFIpass;
    mem.write(1, 3);
  } else if (header.indexOf("?pass_ota=") >= 0) {
    newOTApass = setOption(newOTApass, "pass_ota", "OTA password", 15, 121, 135, true);
  } else if (header.indexOf("?mode_wifi=") >= 0) {
    setOption(mem.read(3), "mode_wifi", "WIFI mode", 0, 1, 3, true);          
  } else if (header.indexOf("?ap_wifi=") >= 0) {
    wi_fi.switchAP = setOption(wi_fi.switchAP, "ap_wifi", "wifi switch exists", 0, 1, 4, false); 
  //FIRMWARE/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?address_update=") >= 0) {
    update_server = setOption(update_server, "address_update", "server address", 15, 151, 165, false);      
  } else if (header.indexOf("?remote_update=") >= 0) {
    mqtt.updateStart = setOption(mqtt.updateStart, "remote_update", "remote update", 0, 1, 168, false);      
  //CLEAR/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("GET /save_clear") >= 0) {
    mem.clear();
    menu = "main";
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////
////        WEB PAGE        ////////        WEB PAGE        ////////        WEB PAGE        ////
////////////////////////////////////////////////////////////////////////////////////////////////
void web_page(WiFiClient &client, String &header) {
  web_createPage(client);
  // Web Page Heading
  client.println("<body><h3>pyramidak air firmware " + firmwareVersion + "</h3>");
  client.println("<h1>" + deviceName + "</h1>");

  //LOCK/////////////////////////////////////////////////////////////////////////////////////
  if (mem.readAndCheck("", "weblock password", 170, 175, true) != lockPass) {
    createSubmit(client, "PASSWORD", lockPass, "", "unlock", 6, "access lock to firmware setting");
    return;
  }   
  //MAIN/////////////////////////////////////////////////////////////////////////////////////
  if (menu == "main") {
    if (qualityFinal != 0) {
      client.println("<p><b>Air Quality is " + qualityWord + "</b></p>");
    }
    if (light.connected) {
      client.println("<p><b>Ambient Light</b></br>");
      client.println("Sensor: " + light.ProductName + "</br>");
      client.println("<b>Light lux: " + String(light.lux) + "</b></p>");
    }
    if (pm.connected) {
      client.println("<p><b>Particle matter</b></br>");
      client.println("Sensor: " + pm.ProductName + "</br>");
      if (pm.sensor == 1) {
        client.println("SN: " + pm.SerialNumber + "</br>");
        client.println("Firmware: " + pm.FirmwareVersion + "</br>");
        client.println("Library: " + pm.LibraryVersion + "</br>");
      }
      if (pm.running) {
        client.println("<b>Particle concentration</b></br>");
        client.println("Mass &#181;g/m&#179;: PM1.0 " + String(pm.values.MassPM1) + "; PM2.5 " + String(pm.values.MassPM2) + "; PM4.0 " + String(pm.values.MassPM4) + "; PM10 " + String(pm.values.MassPM10) + "</br>");
        client.println("Number #/cm&#179;: PM1.0 " + String(pm.values.NumPM1) + "; PM2.5 " + String(pm.values.NumPM2) + "; PM4.0 " + String(pm.values.NumPM4) + "; PM10 " + String(pm.values.NumPM10) + "</br>");
        if (pm.sensor == 1) client.println("Size &#181;m: " + String(pm.values.PartSize));
        client.println("</p>");
      }    
    }
    if (co2.connected) {
      client.println("<p><b>Carbon dioxide</b></br>");
      client.println("Sensor: " + co2.ProductName + "</br>");
      client.println("SN: " + co2.SerialNumber + "</br>");
      client.println("Firmware: " + co2.FirmwareVersion + "</br>");
      client.println("<b>Carbon dioxide ppm: " + String(co2.co2) + "</b></br>");
      client.println("Temperature &#176;C: " + String(co2.temperature) + "</br>");
      client.println("Humidity %: " + String(co2.humidity) + "</p>");
    }
    if (voc.connected) {
      client.println("<p><b>Volatile Organic Compounds</b></br>");
      client.println("Sensor: " + voc.ProductName + "</br>");
      if (voc.LibraryVersion != "") client.println("Library: " + voc.LibraryVersion + "</br>");
      if (voc.sensor == 68) {
        client.println("<b>TVOC index: " + String(voc.tvoc) + "</b></br>");
        client.println("Temperature &#176;C: " + String(voc.temperature) + "</br>");
        client.println("Pressure hPa: " + String(voc.pressure) + "</br>");
        client.println("Humidity %: " + String(voc.humidity) + "</p>");
      } else if (voc.sensor == 30) {
        client.println("<b>TVOC ppb: " + String(voc.tvoc) + "</b></p>");
      } else if (voc.sensor == 10) {
        client.println("<b>TVOC ppm: " + String(voc.tvoc) + "</b></p>");
      }
    } 
    if (termistor.connected) {
      client.println("<p><b>Thermistor</b></br>");
      client.println("Sensor: " + termistor.ProductName + "</br>");
      if (termistor.SerialNumber != "") client.println("SN: " + termistor.SerialNumber + "</br>");
      client.println("<b>Temperature &#176;C: " + String(termistor.temperature) + "</b></br>");
      if (termistor.pressure != 0) {
        client.println("Pressure hPa: " + String(termistor.pressure) + "</br>");
      }
      if (termistor.humidity != 0) {
        client.println("Humidity %: " + String(termistor.humidity) + "</p>");
      }      
    }
    if (termistor1.connected) {
      client.println("<p><b>Thermistor</b></br>");
      client.println("Sensor: " + termistor1.ProductName + "</br>");
      client.println("<b>Temperature &#176;C: " + String(termistor1.temperature) + "</b></br>");
      if (termistor1.humidity != 0) {
        client.println("Humidity %: " + String(termistor1.humidity) + "</p>");
      }      
    }    
    if (mqtt.temperature != -100) {
      client.println("<p><b>Thermistor</b></br>");
      client.println("Sensor: external</br>");
      client.println("<b>Temperature &#176;C: " + String(mqtt.temperature) + "</b></br>");
    }    
    if (wind.connected) {
      client.println("<p><b>Wind</b></br>");
      client.println("Sensor: " + wind.ProductName + "</br>");
      client.println("<b>Speed km/h: " + String(wind.speed) + "</b></p>");
    } 
    if (rain.connected) {
      client.println("<p><b>Moisture</b></br>");
      client.println("Sensor: " + rain.ProductName + "</br>");
      client.println("<b>Rainy %: " + String(rain.percent) + "</b></br>");
      createMenu(client, "SET DRY", "analog/min", false);
      createMenu(client, "SET WET", "analog/max", false);
      client.println("</p>");
    } 
    if (sensorDigital1 >= 1 and sensorDigital1 <= 6) { 
      client.println("<p><b>Logic digital state: " + logic.getState(logic.digital) + "</b></p>");
    }
    if (sensorAnalog >= 1 and sensorAnalog <= 6) {
      client.println("<p><b>Logic analog state: " + logic.getState(logic.analog) + "</b></p>");
    } 
    //MAIN MENU/////////////////////////////////////////////////////////////////////////////////////
    client.println("<p>MAIN MENU</p>");
    if (web_restartNeeded == true) {
      createMenu(client, "Reconnect", "reconnect");
      client.println("<p>RESTART NEEDED</p>");
    }
    createMenu(client, "Reload", "main");
    createMenu(client, "Lock", "lock");
    createMenu(client, "Device", "device");
    createMenu(client, "Sensor", "sensor");
    createMenu(client, "LED", "led");
    createMenu(client, "MQTT", "mqtt");
    createMenu(client, "WIFI", "wifi");
    createMenu(client, "Firmware", "firmware");
    createMenu(client, "Clear", "clear");
    createMenu(client, "Restart", "restart");
  //LOCK/////////////////////////////////////////////////////////////////////////////////////
  } else if (menu == "lock") {
    createSubmit(client, "PASSWORD", lockPass, "", "lock_pass", 6, "access lock to firmware setting");
    
    client.println("<p>LOCK MENU</p>");
    createMenu(client, "Lock", "setlock");
    createMenu(client, "Return", "main");
  //DEVICE/////////////////////////////////////////////////////////////////////////////////////
  } else if (menu == "device") {
    createSubmit(client, "DEVICE PURPOSE", device, "1-2", "purpose", 1, "1-air quality, 2-smoggie pm");
    createSubmit(client, "DEVICE NAME", deviceName, "New name", "name", 15, "");
    
    client.println("<p>DEVICE MENU</p>");
    createMenu(client, "Return", "main");
  //SENSOR/////////////////////////////////////////////////////////////////////////////////////
  } else if (menu == "sensor") {
    createSubmit(client, "ANALOG SENSOR", sensorAnalog, "0-10", "1analog", 2, "0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC TOGGLE, 7-anemometer, 8-lightmeter TEMT6000, 9-raindrops, 10-TVOC MICS5524");                 
    createSubmit(client, "DIGITAL SENSOR", sensorDigital1, "0-9", "1digital", 1, "0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC TOGGLE, thermometer 7-DHT11, 8-DHT22, 9-DS18B20");
    if (wind.connected) createSubmit(client, "wind speed max. m/s", wind.speedMax, "9-99", "wind_speed", 2, "");
    if (co2.connected or termistor.connected or voc.connected) { //temperature and humidity sensor calibration
      if (co2.temperature != 0 or termistor.temperature != 0 or voc.temperature != 0 or termistor1.temperature != 0) {
        createSubmit(client, "temperature correction", String(termistor.fixTemperature*-1), "0.0-9.9", "fix_temp", 3, "");
        createSubmit(client, "temperature index 2", String(termistor.temperatureIndex2), "0-99", "index2temp", 2, "");
        createSubmit(client, "temperature index 3", String(termistor.temperatureIndex3), "0-99", "index3temp", 2, "");
      }
      if (co2.humidity != 0 or termistor.humidity != 0 or voc.humidity != 0 or termistor1.humidity != 0) {
        createSubmit(client, "humidity correction", String(termistor.fixHumidity), "0-9", "fix_humi", 1, "");
        createSubmit(client, "humidity index 2", String(termistor.humidityIndex2), "0-99", "index2humid", 2, "");
        createSubmit(client, "humidity index 3", String(termistor.humidityIndex3), "0-99", "index3humid", 2, "");
      }
      if (termistor.pressure != 0 or voc.pressure != 0) createSubmit(client, "sea level pressure", String(termistor.altitude), "meters", "altitude", 4, "");
    }
    
    client.println("<p>SENSOR MENU</p>");
    createMenu(client, "Return", "main");
    client.println("SENSOR PIN:");
    client.println("<p>Analog A0, Digital D5</p>");
  //LED/////////////////////////////////////////////////////////////////////////////////////                            
  } else if (menu == "led") {
    createSubmit(client, "LED BRIGHTNESS", semafor.brightness(), "1-177", "diodbright", 3, ""); 

    client.println("<p>DIOD MENU</p>");
    createMenu(client, "Return", "main");
    client.println("<p>DIOD PIN:</p>");
    client.println("<p>RED D6, YELLOW D7, GREEN D8, BLUE D4</p>");
  //MQTT/////////////////////////////////////////////////////////////////////////////////////                            
  } else if (menu == "mqtt") {
    createSubmit(client, "BROKER", mqtt.server, "New broker", "server_mqtt", 15, "");
    createSubmit(client, "USER", mqtt.user, "New user", "user_mqtt", 15, "");
    createSubmit(client, "PASSWORD", newMQTTpass, "New pass", "pass_mqtt", 15, "");

    client.println("<p>MQTT MENU</p>");
    createMenu(client, "Return", "main"); 

    web_createMQTTmanual(client);             
  //WIFI///////////////////////////////////////////////////////////////////////////////////// 
  } else if (menu == "wifi") {
    createSubmit(client, "SSID", wi_fi.ssid, "New SSID", "ssid_wifi", 15, "");
    createSubmit(client, "PASSWORD", newWIFIpass, "New pass", "pass_wifi", 15, "");
    createSubmit(client, "NEXT START", mem.read(3), "New mode", "mode_wifi", 1, "0-Access Point, 1(2)-Client");
    createSubmit(client, "WIFI SWITCH", wi_fi.switchAP, "Exists", "ap_wifi", 1, "0-none, 1-WiFi AP");

    client.println("<p>WIFI MENU</p>");
    createMenu(client, "Return", "main");  
    client.println("switch PIN: D0");
  //FIRMWARE///////////////////////////////////////////////////////////////////////////////////// 
  } else if (menu == "firmware") {
    createSubmit(client, "UPDATE SERVER", update_server, "http://", "address_update", 15, "");
    createSubmit(client, "UPDATE START", mqtt.updateStart, "0-1", "remote_update", 15, "0 - manual only, 1 - mqtt command");
    createSubmit(client, "ARDUINO UPDATE", newOTApass, "New pass", "pass_ota", 15, "");

    client.println("<p>FIRMWARE MENU</p>");                                                     
    createMenu(client, "Return", "main");                             

    if (update_server != "") {
      if (header.indexOf("GET /update") >= 0) {
        client.println("<h3>update in progress...</br>");  
        client.println("...click on Return in a minute</h3>"); 
        client.println("<p>update " + update_Server_begin() + "</p>");    
      } else {
        String newVersion = update_Server_version();
        if (newVersion.length() > 5) {
          client.println("<h3>" + newVersion + "</h3>");                         
          newFirmwareVersion = "";
        } else { 
          newFirmwareVersion = newVersion;         
        }
        if (newFirmwareVersion != "") {
          if(newFirmwareVersion != firmwareVersion) {createMenu(client, "Update", "update");}
          client.println("<h3>firmware version on server " + newFirmwareVersion + "</h3>");
        }
      } 
    }          
  //CLEAR///////////////////////////////////////////////////////////////////////////////////// 
  } else if (menu == "clear") {
    client.println("<p>CLEAR MEMORY SETTINGS</p>");
    createMenu(client, "Confirm", "save_clear");
    createMenu(client, "Return", "main");                            
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////
////       MQTT MANUAL      ////////       MQTT MANUAL      ////////       MQTT MANUAL      ////
////////////////////////////////////////////////////////////////////////////////////////////////
void web_createMQTTmanual(WiFiClient client) {
  client.println("<p>Topics for central system of</br>");
  client.println("smart home as Home Assistant</p>");
  
  client.println("<p>DEVICE TOPICS</p>");
  
  client.println("<p>HA configuration:</br>");
  client.println("platform: mqtt</br>");

  client.println("<p>Commands:</br>");
  client.println("\"" + deviceName + "/restart/command\"</br>");
  client.println("\"" + deviceName + "/update/command\"</br>");
  client.println("\"" + deviceName + "/brightness/command\"</br>");
  client.println("\"" + deviceName + "/temperature/command\" (external thermistor)</br>");

  client.println("States:</br>");
  client.println("\"" + deviceName + "/status\"</br>");
  client.println("\"" + deviceName + "/ip/state\"</br>");
  client.println("\"" + deviceName + "/update/state\"</br>");
  client.println("\"" + deviceName + "/logic/analog\"</br>");
  client.println("\"" + deviceName + "/logic/digital\"</br>");

  client.println("\"" + deviceName + "/particles/mass\"</br>");
  client.println("\"" + deviceName + "/particles/number\"</br>");
  client.println("\"" + deviceName + "/quality/number\"</br>");
  client.println("\"" + deviceName + "/quality/word\"</br>");
  client.println("\"" + deviceName + "/co2\"</br>");
  client.println("\"" + deviceName + "/temperature\"</br>");
  client.println("\"" + deviceName + "/humidity\"</br>");
  client.println("\"" + deviceName + "/pressure\"</br>");
  client.println("\"" + deviceName + "/tvoc\"</br>");
  client.println("\"" + deviceName + "/light\"</br>");

  client.println("<p>GLOBAL TOPICS</p>");

  client.println("<p>HA start - update devices states:</br>");
  client.println("\"pyramidak/ip/command\"</br>");
  client.println("\"pyramidak/update/command\"</br>");
  client.println("\"pyramidak/sensor/command\"</br>");
}