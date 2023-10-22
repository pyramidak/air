class cWiFi {

private:
  
  #define pinAP D0 //WiFi AP switch
  const byte pinBlue = D4;   //D4=02 on-board diod
  unsigned long lastReconnect;  //wifi connection try
  int wifix;       //state and tries to connect to wifi
  Memory *mem;
  bool connectedJob = true; //do jobs once when connected

  void device(String deviceName_ = "") {
    report("Device: " + deviceName_, true);
    uint8_t mac[6];
    WiFi.macAddress(mac);
    String macStr = macToStr(mac);
    macStr.toUpperCase();
    report("MAC " + macStr);
    if (deviceName_ != "") WiFi.hostname(deviceName_.c_str());
  }

  void disconnect() {
    if (APmode == true) {
      WiFi.softAPdisconnect();
      report("WiFi AP disconnected, stations: " + String(WiFi.softAPgetStationNum()));
    } else {
      WiFi.disconnect();
      report("WiFi disconnected, status: " + String(WiFi.status()));
    }
  }

  void reconnect() {
    reconnectNeeded = false;
    disconnect();
    if (mem->read(3) == 0) {
      beginAP();
    } else {    
      begin(deviceName);
    }
  }

  void sleep() {
    if (digitalRead(pinAP) == LOW and sleeping == true) {   
      sleeping = false;
      WiFi.forceSleepWake();
      delay(10);
      report("modem sleep: OFF", true);
      reconnect();   
    } else if (digitalRead(pinAP) == HIGH and sleeping == false) { 
      sleeping = true;
      disconnect();
      WiFi.forceSleepBegin();
      delay(10); 
      report("modem sleep: ON", true);   
    }
  }

  void led(bool state) {
    if (LEDextra == 0) return;
    if (state == true) {
      if (LEDextra == 2) {
        analogWrite(pinBlue, 177);
      } else { 
        digitalWrite(LED_BUILTIN, LOW);
      }
    } else {
      if (LEDextra == 2) {
        analogWrite(pinBlue, 0);
      } else { 
        digitalWrite(LED_BUILTIN, HIGH);
      }
    }
  }

  void report(String msg, bool offset = false) {
    if(offset) {Serial.println("");}
    Serial.println(msg);
  }

  String macToStr(const uint8_t* mac){
    String result;
    for (int i = 0; i < 6; ++i) {
      result += String(mac[i], 16);
      if (i < 5){
        result += ':';
      }
    }
    return result;
  }

public:
  String ssid       = "";
  String password   = "";
  int switchAP; //Wifi switch AP or modem-sleep exists
  int LEDextra = 1; //0 = none, 1 = board-led, 2 = external-led
  bool APmode;
  bool sleeping;
  bool reconnectNeeded; // force reconnect
  String deviceName;  

  cWiFi(Memory *eprom) {
    mem = eprom;
  }

  bool connected() {
    if (switchAP == 2) sleep();
    if (sleeping == true) return false;
    if (reconnectNeeded == true) reconnect();
    if ((APmode == false and WiFi.status() == WL_CONNECTED) or (APmode == true and WiFi.softAPgetStationNum() > 0)) {
      return true;
    } else {
      if (APmode == false) {
        if (millis() - lastReconnect >= 1000UL or lastReconnect == 0) {
          lastReconnect = millis();
          Serial.print(".");
          if (wifix < 3600) wifix += 1;
          if (wifix == 15 and mem->read(3) != 2) {
            mem->write(0, 3);
            report("WiFi status " + String(WiFi.status() + "."));
            if (String(WiFi.status()) == "7") report("IP not provided, router restart needed.");
          }
          //1:30 trvá restart modemu WIFI //WiFi.printDiag(Serial);
          led(true);
        }
      }
      connectedJob = true;
      return false;
    }    
  }
   
  bool connectedJobs() { //do jobs once when connected
    if (connectedJob == true) {
      connectedJob = false;
      report("WiFi connected.");
      if (APmode == false) {
        report("IP address: " + WiFi.localIP().toString());  
        report("Connection created in " + String(wifix) + " seconds");
        mem->write(2, 3);    
      } else {
        report("IP address: " + WiFi.softAPIP().toString());  
      }
      led(false);
      return true;
    } else {
      return false;
    }    
  }

  void begin(String deviceName_) {
    deviceName = deviceName_;
    APmode = false;
    wifix = 0;
    device(deviceName);
    report("WIFI connecting to: " + ssid);
    WiFi.begin(ssid.c_str(), password.c_str()); 
    led(true);
  }

  void beginAP() {
    APmode = true;
    led(false);
    device();
    report("WIFI AP creating...");
    while (!WiFi.softAP("pyramidak", "")) {
      report("WIFI AP failed:");
      WiFi.printDiag(Serial);
      delay(3000);
    }
    report("WIFI AP ready: pyramidak"); 
    led(true); 
  }

};