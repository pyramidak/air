#include <ArduinoOTA.h> 
#include <ESP8266httpUpdate.h>
#include <ESP8266httpClient.h>

void update_Arduino_handle() {
  ArduinoOTA.handle();    
}

void update_Arduino_begin() {
  //ArduinoOTA.setPort(8266); // Port defaults to 8266
  ArduinoOTA.setHostname(deviceName.c_str());
  if (OTA_password.c_str()) {
    ArduinoOTA.setPassword(OTA_password.c_str());
  }

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    mqtt.disconnect();
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void update_Server_check() {
  mqtt.callUpdate = false;
  if (update_server != "") {
    String newVersion = update_Server_version();
    if (newVersion.length() > 5) {
      mqtt.update(newVersion); //report error version check
    } else { 
      if (newVersion != "") {
        if (newVersion == firmwareVersion) {   
          mqtt.update(firmwareVersion);     
        } else {
          mqtt.update(update_Server_begin());     
        }   
      }      
    }
  }
}

String update_Server_version()  {
  WiFiClient wifiClient;
  HTTPClient httpClient;

  if (!httpClient.begin(wifiClient,"http://" + update_server + ":3000/firmware/version_air.txt")) {
    Serial.println("Connection to version file failed.");
    return "server sonnection failed";
  }    
  int statusCode = httpClient.GET();
  if (statusCode < 0) {
    Serial.printf("GET failed, error: %s\n", httpClient.errorToString(statusCode).c_str());    
    return httpClient.errorToString(statusCode).c_str();
  }
  return httpClient.getString();
}

String update_Server_begin()  {
  mqtt.disconnect();
  WiFiClient client;  

  // The line below is optional. It can be used to blink the LED on the board during flashing
  // The LED will be on during download of one buffer of data from the network. The LED will
  // be off during writing that buffer to flash
  // On a good connection the LED should flash regularly. On a bad connection the LED will be
  // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
  // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

  // Add optional callback notifiers
  ESPhttpUpdate.onStart(update_started);
  ESPhttpUpdate.onEnd(update_finished);
  ESPhttpUpdate.onProgress(update_progress);
  ESPhttpUpdate.onError(update_error);

  t_httpUpdate_return ret = ESPhttpUpdate.update(client, update_server, 3000, "/firmware/pyramidak_air.bin");

  switch (ret) {
    case HTTP_UPDATE_FAILED: Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); break;
    case HTTP_UPDATE_NO_UPDATES: Serial.println("HTTP_UPDATE_NO_UPDATES"); break;
    case HTTP_UPDATE_OK: Serial.println("HTTP_UPDATE_OK"); break;
  }
  
  switch (ret) {
    case HTTP_UPDATE_FAILED: return ESPhttpUpdate.getLastErrorString().c_str();
    case HTTP_UPDATE_NO_UPDATES: return "no update found";
    case HTTP_UPDATE_OK: return "update in progress..."; 
    default: return "unknown result";
  } 
}

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}
void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}
void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}
void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}