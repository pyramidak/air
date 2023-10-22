class AnalogPercent {
  
private:
  #define pinA A0  //analog sensor
  unsigned long lastRead;
  int readingLast = -1;

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

  void report(String msg, bool offset = false) {
    if(offset) {Serial.println("");}
    Serial.println(msg);
  }

public:
  bool connected;
  int reading;
  int percent;
  int min = 50;
  int max = 200;
  bool changed;
  String ProductName = "Raindrops";

  AnalogPercent() {}

  void begin(int sensor) {
    readingLast = -1;
    if (sensor == 9) {
      connected = true;
    } else {
      connected = false;
    }
  }

  void loop() {
    changed = false;
    if (connected == true and (millis() - lastRead >= 500UL or lastRead == 0)) {
      lastRead = millis();
      changed = measure();
    }
  }

  bool measure() {
    if (connected == true) {     
      
      int sample = 10;
      int averageVals[sample];
      for (int i = 0; i < sample; i++) {
        averageVals[i] = analogRead(pinA);
      }
      reading = avarage(averageVals, sample, false);
      percent = float(reading - min) / (max - min) * 100;

      if (abs(reading - readingLast) > 10 or readingLast == -1) {
        readingLast = reading;
        report("analog reading: " + String(reading));
        return true; 
      }
    }
    return false;
  }

};