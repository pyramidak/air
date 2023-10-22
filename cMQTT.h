#include <PubSubClient.h> //mqtt
#include <sps30.h> //struct sps_values
//#pragma once

class cMQTT {

private:
  WiFiClient *client;
  PubSubClient mqtt;
  String firmwareVersion;
  unsigned long lastReport;//time of last mqtt connection status report
  unsigned long lastConnect;  //mqtt connection try
  // MQTT TOPICs //
  String device_update_global = "pyramidak/update/command";
  String sensor_global        = "pyramidak/sensor/command";
  String mqtt_status          = "/status";
  String device_restart       = "/restart/command";
  String device_update_command= "/update/command";
  String device_update_state  = "/update/state";
  String device_ip_state      = "/ip/state";
  String brightness_command   = "/brightness/command";
  String logic_analog         = "/logic/analog";
  String logic_digital        = "/logic/digital";
  String particles_mass       = "/particles/mass";
  String particles_number     = "/particles/number";
  String quality_number       = "/quality/number";
  String quality_word         = "/quality/word";
  String carbon_dioxide       = "/co2";
  String temperature_state    = "/temperature";
  String temperature_command  = "/temperature/command";
  String humidity_state       = "/humidity";
  String pressure_state       = "/pressure";
  String volatile_compound    = "/tvoc";
  String ambient_light        = "/light";

  bool connected() {
    return mqtt.connected();
  }

  void connect() {
    // Attempt to connect
    if (mqtt.connect(deviceName.c_str(), user.c_str(), password.c_str())) {
      mqtt.publish((deviceName + mqtt_status).c_str(), "offline");
      report("MQTT was offline."); 

      mqtt.subscribe((deviceName + temperature_command).c_str());
      report("remote thermistor subscribed");
      
      mqtt.subscribe((deviceName + device_restart).c_str());
      report("restart subscribed");

      mqtt.subscribe((deviceName + device_update_command).c_str());
      mqtt.subscribe(device_update_global.c_str());
      update(firmwareVersion); 
      report("update subscribed");

      reportSensors = true;
      mqtt.subscribe(sensor_global.c_str());
      report("sensors subscribed");

      mqtt.subscribe((deviceName + brightness_command).c_str());
      report("brightness subscribed");

      mqtt.publish((deviceName + mqtt_status).c_str(), "online");
      report("MQTT connected to " + server, true);  
  
    } else {
      report("MQTT connn.failed: " + String(mqtt.state()));
    }
  }

  void report(String msg, bool offset = false) {
    if(offset) {Serial.println("");}
    Serial.println(msg);
  }

public:
  // MQTT settings //
  String server     = "192.168.0.181";
  String user       = "";//mqtt_pc";
  String password   = "";//photosmart";
  String deviceName;
  bool reportSensors;
  bool callUpdate;
  bool reconnectNeeded; // force reconnect
  int diodBrightChange;
  int updateStart;      //web server update start: 0 - manual only, 1 - mqtt command
  float temperature = -100; //remote thermistor
  bool reportTemp; //report remote thermistor

  cMQTT() {}

  void begin(String deviceName_, String firmwareVersion_) {
    deviceName = deviceName_;
    firmwareVersion = firmwareVersion_;
    client = new WiFiClient();
    //mqtt = new PubSubClient();
    mqtt.setClient(*client);
    mqtt.setServer(server.c_str(), 1883);
    mqtt.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
  }

  void callback(char* topic, byte* payload, unsigned int length) {
    //convert topic to string to make it easier to work with
    String topicStr = topic; 
    String payloadStr = "";
    for (int i = 0; i < length; i++) { payloadStr = payloadStr + (char)payload[i]; }
      
    report("Message arrived:", true);  
    report("topic: " + topicStr);  
    report("payload: " + payloadStr);  

    if(topicStr == (deviceName + device_restart)) {
      disconnect();
      ESP.restart();
    }

    if(topicStr == (deviceName + device_update_command) or topicStr == device_update_global) {
      if (updateStart == 1) callUpdate = true;
    }

    if(topicStr == sensor_global) {
      update(firmwareVersion);
      reportSensors = true; 
    }

    if(topicStr == (deviceName + brightness_command)) diodBrightChange = payloadStr.toInt(); 
    
    if(topicStr == (deviceName + temperature_command) and payloadStr != "") {
      if (temperature == -100) {
        if (payloadStr.toFloat() != 0) temperature = payloadStr.toFloat();
      } else {
        temperature = payloadStr.toFloat();
      }    
      reportTemp = true;
    }
  }

  void disconnect() {
    reconnectNeeded = false;
    lastReport = 0;
    lastConnect = 0;
    mqtt.publish((deviceName + mqtt_status).c_str(), "offline");
    report("MQTT status: offline");
    mqtt.disconnect();
    mqtt.setServer(server.c_str(), 1883);
  }

  void loop() {
    if (reconnectNeeded == true) disconnect();
    if (mqtt.connected() == true) {
      if (millis() - lastReport >= 60*1000UL or lastReport == 0) {  
        lastReport = millis();  
        mqtt.publish((deviceName + mqtt_status).c_str(), "online");
        report("MQTT status: online");
      }
      mqtt.loop();    
    } else {
      if (millis() - lastConnect >= 10*1000UL or lastConnect == 0) {
        lastConnect = millis();
        connect();
      }
    }
  }

  void update(String version) { 
    mqtt.publish((deviceName + device_ip_state).c_str(), WiFi.localIP().toString().c_str()); 
    mqtt.publish((deviceName + device_update_state).c_str(), version.c_str());
    report("update state and IP address published: " + version);            
  }

  void quality(int quality, String word) { 
    if (mqtt.connected() == true and quality != 0) { 
      mqtt.publish((deviceName + quality_number).c_str(), String(quality).c_str());    
      report("air quality score published: " + String(quality));
      mqtt.publish((deviceName + quality_word).c_str(), String(word).c_str());    
      report("air quality word published: " + String(word));
    }
  }

  void voc(int tvoc) { 
    if (mqtt.connected() == true and tvoc != 0) {     
      mqtt.publish((deviceName + volatile_compound).c_str(), String(tvoc).c_str());    
      report("TVOC published: " + String(tvoc));
    }
  }

  void co2(int co2) { 
    if (mqtt.connected() == true) { 
      if (co2 != 0) { 
        mqtt.publish((deviceName + carbon_dioxide).c_str(), String(co2).c_str());    
        report("carbon dioxide published: " + String(co2));
      } 
    }
  }

  void temp(float temperature, int humidity, int pressure = 0) { 
    if (mqtt.connected() == true) { 
      if (temperature != 0) { 
        mqtt.publish((deviceName + temperature_state).c_str(), String(temperature).c_str());    
        report("temperature published: " + String(temperature));
      }
      if (humidity != 0) { 
        mqtt.publish((deviceName + humidity_state).c_str(), String(humidity).c_str());    
        report("humidity published: " + String(humidity));
      }
      if (pressure != 0) { 
        mqtt.publish((deviceName + pressure_state).c_str(), String(pressure).c_str());    
        report("pressure published: " + String(pressure));
      }
    }
  }

  void pm(sps_values values) { 
    if (mqtt.connected() == true and values.PartSize != 0) { 
      String payload = "{\"PM1\":\"" + String(values.MassPM1) + "\",";
      payload = payload + "\"PM25\":\"" + String(values.MassPM2) + "\",";
      payload = payload + "\"PM4\":\"" + String(values.MassPM4) + "\",";
      payload = payload + "\"PM10\":\"" + String(values.MassPM10) + "\"}";
    
      mqtt.publish((deviceName + particles_mass).c_str(), payload.c_str());    
      report("particle mass concentration published:");
      report(payload);

      payload = "{\"PM1\":\"" + String(values.NumPM1) + "\",";
      payload = payload + "\"PM25\":\"" + String(values.NumPM2) + "\",";
      payload = payload + "\"PM4\":\"" + String(values.NumPM4) + "\",";
      payload = payload + "\"PM10\":\"" + String(values.NumPM10) + "\"}";
    
      mqtt.publish((deviceName + particles_number).c_str(), payload.c_str());    
      report("particle number concentration published:");
      report(payload);
    }
  }

  void light(int value) {
    if (mqtt.connected() == true and value > 0) { 
      mqtt.publish((deviceName + ambient_light).c_str(), String(value).c_str());
      report("ambient light published: " + String(value));
    }
  }

  void analog(int state) {  
    analog(String(state));
  }

  void analog(String state) {  
    if (mqtt.connected() == true) { 
      mqtt.publish((deviceName + logic_analog).c_str(), state.c_str()); 
      report("analog sensor state published: " + state);
    }
  }

  void digital(String state) { 
    if (mqtt.connected() == true) { 
      mqtt.publish((deviceName + logic_digital).c_str(), state.c_str()); 
      report("digital sensor state published: " + state);
    }
  }

};