class Anemometer {

private:
  #define pinA A0  //analog sensor
  unsigned long lastRead;
  int speedLast = -1;

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

public:
  String ProductName = "Anemometer";
  bool connected;
  int speed;
  int speedMax = 30;
  bool changed;

  Anemometer() {}

  void begin(int sensor) {    
    if (sensor == 7) {
      connected = true;
    } else {
      connected = false;
      speedLast = -1;
    }
  }

  void loop() {
    changed = false;
    if (connected == true and (millis() - lastRead >= 1000UL or lastRead == 0)) {
      lastRead = millis();
      changed = measure();
    }
  }

  bool measure() {
    if (connected == true) {     
      int sampleLength = 10;
      int averageVals[sampleLength];
      for (int i = 0; i < sampleLength; i++) {
        averageVals[i] = analogRead(pinA);
      }
      speed = round(float(speedMax) / 1024 * avarage(averageVals, sampleLength, false) * 3.6); //1 m/s = 3,6 km/h
      
      if (abs(speed - speedLast) > 4 or speedLast == -1) {
        speedLast = speed;
        return true;
      }
    }
    return false;
  }

};