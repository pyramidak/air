#include <sps30.h>
#include <SoftwareSerial.h>

/************************************************************************************
//  AUTHOR: Paul van Haastrecht
//    DATE: January 2019 - July 2020
// PURPOSE: Reading particle matter sensor SPS30
//  AUTHOR: paulvha
//    DATE: august 2018 - january 2019
//   ADDED: support ESP8266 and ESP32
//     URL: https://github.com/paulvha/sps30
/************************************************************************************/
class ParticleMatter {

private:
  bool reportPlantower = false;
/////////////////////////////////////////////////////////////
/*define communication channel to use for SPS30
 valid options:
 *   I2C_COMMS              use I2C communication
 *   SOFTWARE_SERIAL        Arduino variants (NOTE) 
 *   SERIALPORT             ONLY IF there is NO monitor attached
 *   SERIALPORT1            Arduino MEGA2560, Due.  Sparkfun ESP32 Thing : MUST define new pins as defaults are used for flash memory)
 *   SERIALPORT2            Arduino MEGA2560, Due and ESP32
 *   SERIALPORT3            Arduino MEGA2560 and Due only for now

 * ESP8266 with UART not tested as the power is only 3V3 (the SPS30 needs 5V) and one has to use SOFTWARE_SERIAL.
 * SOFTWARE_SERIAL 9600 has been left in as an option, but as the SPS30 is only working on 115200, so the connection will probably NOT work on any device. */ 
  #define SP30_COMMS I2C_COMMS 
/////////////////////////////////////////////////////////////
/* define RX and TX pin for softserial and Serial1 on ESP32
 * can be set to zero if not applicable / needed           */
  #define TX_PIN 26
  #define RX_PIN 25
/////////////////////////////////////////////////////////////
/* define driver debug
 * 0 : no messages
 * 1 : request sending and receiving
 * 2 : request sending and receiving + show protocol errors */
  #define DEBUG 0
 //////////////////////////////////////////////////////////////
  SPS30 sps;
  int noDetect;
  unsigned long lastRead;
  unsigned long lastDetect;
  struct sps_values valuesLast;
  //////////////////////////////////////////////////////////////
  //PLANTOWER
  SoftwareSerial *pmsSerial;

  struct pmsData {
    uint16_t framelen;
    uint16_t pm10_standard, pm25_standard, pm100_standard;
    uint16_t pm10_env, pm25_env, pm100_env;
    uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
    uint16_t unused;
    uint16_t checksum;
  };
  struct pmsData data;

  boolean readPMSdata(Stream *s) {
    if (! s->available()) {
      if (reportPlantower) Serial.println(F("Plantower stream not available."));
      return false;
    }
    
    // Read a byte at a time until we get to the special '0x42' start-byte
    //if ord(serialPort.read()) == 0x42 and ord(serialPort.read()) == 0x4d:
    if (s->peek() != 0x42) {
      if (reportPlantower) Serial.println(F("Plantower start-byte error."));
      s->read();
      return false;
    }
  
    // Now read all 32 bytes
    if (s->available() < 32) {
      if (reportPlantower) Serial.println(F("Plantower data not received."));
      return false;
    }
      
    uint8_t buffer[32];    
    uint16_t sum = 0;
    s->readBytes(buffer, 32);
  
    // get checksum ready
    for (uint8_t i=0; i<30; i++) {
      sum += buffer[i];
    }
  
    /* debugging
    for (uint8_t i=2; i<32; i++) {
      Serial.print("0x"); Serial.print(buffer[i], HEX); Serial.print(", ");
    }
    Serial.println();
    */
    
    // The data comes in endian'd, this solves it so it works on all platforms
    uint16_t buffer_u16[15];
    for (uint8_t i=0; i<15; i++) {
      buffer_u16[i] = buffer[2 + i*2 + 1];
      buffer_u16[i] += (buffer[2 + i*2] << 8);
    }
  
    // put it into a nice struct :)
    memcpy((void *)&data, (void *)buffer_u16, 30);
  
    if (sum != data.checksum) {
      if (reportPlantower) Serial.println("Plantower checksum failure.");
      return false;
    }
    // success!
    if (reportPlantower) Serial.println("Plantower reading success!");
    s->flush();
    return true;
  }
  
  void qualityCalculation() {
    if (values.MassPM1 > 55 or values.MassPM2 > 70 or values.MassPM4 > 85 or values.MassPM10 > 100) {
      quality = 3;
    } else if (values.MassPM1 > 20 or values.MassPM2 > 30 or values.MassPM4 > 40 or values.MassPM10 > 50) {
      quality = 2;
    } else {
      quality = 1;
    } 
  }

  void GetDeviceInfo()
  {
    char buf[32];
    uint8_t ret;
    SPS30_version v;

    //try to read serial number
    ret = sps.GetSerialNumber(buf, 32);
    if (ret == SPS30_ERR_OK) {
      Serial.print(F("Serial number : "));
      if (strlen(buf) > 0) {
        Serial.println(buf);
        SerialNumber = String(buf);
      } else { 
        Serial.println(F("not available"));
      }
    } else { 
      ErrtoMess((char *) "could not get serial number", ret);
    }

    // try to get product name
    ret = sps.GetProductName(buf, 32);
    if (ret == SPS30_ERR_OK)  {
      Serial.print(F("Product name  : "));
      if (strlen(buf) > 0) {
        Serial.println(buf);
      } else { 
        Serial.println(F("not available"));
      }
    }
    else
      ErrtoMess((char *) "could not get product name.", ret);

    // try to get version info
    ret = sps.GetVersion(&v);
    if (ret != SPS30_ERR_OK) {
      Serial.println(F("Can not read version info"));
      return;
    }

    Serial.print(F("Firmware level: "));  Serial.print(v.major);
    Serial.print("."); Serial.println(v.minor);
    FirmwareVersion = String(v.major) + "." + String(v.minor);

    if (SP30_COMMS != I2C_COMMS) {
      Serial.print(F("Hardware level: ")); Serial.println(v.HW_version);

      Serial.print(F("SHDLC protocol: ")); Serial.print(v.SHDLC_major);
      Serial.print("."); Serial.println(v.SHDLC_minor);
    }

    Serial.print(F("Library level : "));  Serial.print(v.DRV_major);
    Serial.print(".");  Serial.println(v.DRV_minor);
    LibraryVersion = String(v.DRV_major) + "." + String(v.DRV_minor);
  }

  void Errorloop(char *mess, uint8_t r)
  {
    if (r) ErrtoMess(mess, r);
    else Serial.println(mess);
  }

  void ErrtoMess(char *mess, uint8_t r)
  {
    char buf[80];

    Serial.print(mess);

    sps.GetErrDescription(r, buf, 80);
    Serial.println(buf);
  }

  /**
  * serialTrigger prints repeated message, then waits for enter
  * to come in from the serial port.
  */
  void serialTrigger(char * mess)
  {
    Serial.println();

    while (!Serial.available()) {
      Serial.println(mess);
      delay(2000);
    }

    while (Serial.available())
      Serial.read();
  }

public:
  String SerialNumber;
  String ProductName;
  String FirmwareVersion;
  String LibraryVersion;
  bool connected;
  bool running;
  struct sps_values values;
  int interval; //read interval in seconds
  int quality; //quality of air
  int sensor;
  bool changed;

  ParticleMatter(int timeInterval = 3) {
    interval = timeInterval;
  }

  void begin(SoftwareSerial *serial) {
    //PLANTOWER
    if (connected == false) {
      noDetect = 0;
      pmsSerial = serial;
      pmsSerial->begin(9600);  
    }
  }

  void detect() {
    if (connected == false and noDetect < 5) {
      if (millis() - lastDetect >= 5*1000UL or lastDetect == 0) {
        lastDetect = millis();
        noDetect += 1;
        if (readPMSdata(pmsSerial)) {
          Serial.println(F("PMSx003 connected."));
          ProductName = "PMSx003";
          connected = true;
          running = true;
          sensor = 2;
        }
      }
    }
  }

  void begin() {
    //SENSIRION
    // set driver debug level
    sps.EnableDebugging(DEBUG);
    // set pins to use for softserial and Serial1 on ESP32
    if (TX_PIN != 0 && RX_PIN != 0) sps.SetSerialPin(RX_PIN,TX_PIN);
    // Begin communication channel;
    if (! sps.begin(SP30_COMMS)) Errorloop((char *) "could not initialize communication channel.", 0);
    // check for SPS30 connection
    Serial.print("\r\n");
    if (! sps.probe()) {
      Errorloop((char *) "SPS30 Not connected.", 0);
      connected = false;
    } else {
      Serial.println(F("SPS30 connected."));
      ProductName = "SPS30";
      connected = true;
      sensor = 1;
      // reset SPS30 connection
      if (! sps.reset()) Errorloop((char *) "could not reset.", 0);
      // read device info
      GetDeviceInfo();
      if (SP30_COMMS == I2C_COMMS) {
        if (sps.I2C_expect() == 4)
          Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
      }
    }
  }

  void start() {
    if (sensor != 1) return;
    // start measurement
    if (connected == false or running == true) return;
    if (sps.start()) {
      Serial.println(F("SPS30 started"));
      running = true;
    } else {
      Errorloop((char *) "SPS30 could NOT start", 0);
      running = false;
    }
  }  

  void stop() {
    if (sensor != 1) return;
    // stop measurement
    if (connected == false or running == false) return;
    if (sps.stop()) {
      Serial.println(F("Measurement stopped"));
      running = false;
    } else {
      Errorloop((char *) "Could NOT stop measurement", 0);
      running = true;
    }
  }  

  void loop() {
    changed = false;
    if (connected == true and running == true) {
      if (millis() - lastRead >= interval*1000UL or lastRead == 0) {
        lastRead = millis();
        changed = read();
      }
    } 
  }

  bool read() {
    if (!connected) return false;
    static bool header = true;
    uint8_t ret, error_cnt = 0;
    struct sps_values val;

    //PLANTOWER
    if (sensor == 2) {
      if (readPMSdata(pmsSerial)) {
        // reading data was successful!
        values.MassPM1 = data.pm10_standard; //pm10_env
        values.MassPM2 = data.pm25_standard; //pm25_env
        values.MassPM4 = 0;
        values.MassPM10 = data.pm100_standard; //pm100_env
        values.NumPM1 = data.particles_10um;
        values.NumPM2 = data.particles_25um;
        values.NumPM4 = data.particles_50um;
        values.NumPM10 = data.particles_100um;
      } else {
        return false;
      } 
    }

    // SENSIRION
    if (sensor == 1) {
      do {
        ret = sps.GetValues(&val);

        // data might not have been ready
        if (ret == SPS30_ERR_DATALENGTH){

            if (error_cnt++ > 3) {
              ErrtoMess((char *) "Error during reading values: ",ret);
              return false;
            }
            delay(1000);
        }

        // if other error
        else if(ret != SPS30_ERR_OK) {
          ErrtoMess((char *) "Error during reading values: ",ret);
          return false;
        }
      } while (ret != SPS30_ERR_OK);

      values = val;
      values.NumPM1 = val.NumPM1 - val.NumPM0;
      values.NumPM2 = val.NumPM2 - val.NumPM1;
      values.NumPM4 = val.NumPM4 - val.NumPM2;
      values.NumPM10 = val.NumPM10 - val.NumPM4;
    }
    
    if (abs(values.MassPM1 - valuesLast.MassPM1) > 6 or abs(values.NumPM2 - valuesLast.NumPM2) > 3 or abs(values.NumPM4 - valuesLast.NumPM4) > 2 or abs(values.NumPM10 - valuesLast.NumPM10) > 1) {
      valuesLast = values;
      qualityCalculation();
      return true;
    } else {
      return false;
    }    
  }

};
