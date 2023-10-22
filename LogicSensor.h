class LogicSensor {

  private:
    #define pinA  A0 //analog sensor
    const byte pinD1 = D5;  
    int potentio;

  public:
    bool digital;   //digital sensor state
    bool analog;    //analog0 sensor state

    LogicSensor() {}

    bool readDigital(int sensorDigital) {
      if (sensorDigital >= 1 and sensorDigital <= 6) {
        int ON, OFF;
        if(sensorDigital >= 1 and sensorDigital <= 3) {
          ON = 1;
          OFF = 0;
        } else {
          ON = 0;
          OFF = 1;
        } 
        if (digitalRead(pinD1) == ON and digital == false) {
          Serial.print("Digital switch: ON"); 
          digital = true;
          return true;
        } else if (digitalRead(pinD1) == OFF and digital == true) {
          Serial.print("Digital switch: OFF"); 
          digital = false;
          return true;
        }
      }
      return false;
    }

    bool readAnalog(int sensorAnalog) {
      if (sensorAnalog >= 1 and sensorAnalog <= 6) {
        int p = analogRead(pinA) / 4.01;
        if (sensorAnalog >= 1 and sensorAnalog <= 3) {
          if (p > potentio + 200) {
            potentio = p; 
            Serial.print("Analog switch: ON"); 
            analog == true; 
            return true;     
          } else if (p < potentio - 200) {
            potentio = p;
            Serial.print("Analog switch: OFF"); 
            analog == false;
            return true;
          }
        } else if (sensorAnalog >= 4 and sensorAnalog <= 6) {
          if (p < potentio - 200) {
            potentio = p;
            Serial.print("Analog switch: ON"); 
            analog == true;
            return true;
          } else if (p > potentio + 200) {
            potentio = p;
            Serial.print("Analog switch: OFF"); 
            analog == false;
            return true;
          }
        }
      }
      return false;
    }

    String getState(bool value) {
      if (value == true) {
        return "ON";
      } else {
        return "OFF";
      }  
    }

};