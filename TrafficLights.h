class TrafficLights {

private:
   // PINs usage declaration //
  const byte pinRed    = D6;   //D6=12
  const byte pinYellow = D7;   //D7=13  
  const byte pinGreen  = D8;   //D8=14
  int diodBright = diodLimit;  //current brightness of semafor lights

public:
  const int diodLimit = 177;   //max current limit of leds
  int red, yellow, green;      //brightness of semafor lights
  int diodMax = diodLimit;     //max allowed brightness

  TrafficLights() {}

  void begin() {
    pinMode(pinRed, OUTPUT); // onboard Led light
    pinMode(pinYellow, OUTPUT); // onboard Led light
    pinMode(pinGreen, OUTPUT); // onboard Led light
    diodBright = diodMax;
  }

  void light() {
    analogWrite(pinRed, red);
    analogWrite(pinYellow, yellow);  
    analogWrite(pinGreen, green);
  }

  int brightness(int value = -1) {
    if (value < 0) return diodBright;
    if (value > diodLimit) value = diodLimit;
    if (value < 1) value = 1;
    diodBright = value;
    if (red != 0) red = diodBright;
    if (yellow != 0) yellow = diodBright;
    if (green != 0) green = diodBright;
    light();
    return diodBright;
  }

  void light(String color, int value) {
    if (value > diodLimit) value = diodLimit;
    if (value < 1) value = 1;
    red = 0;
    yellow = 0;
    green = 0;
    if (color == "red") {
      red = value;
    } else if (color == "yellow") {
      yellow = value;
    } else if (color == "green") {
      green = value;  
    }
    light();
  }

  void light(int color) {
    red = 0;
    yellow = 0;
    green = 0;
    if (color == 3) {
      red = diodBright;
    } else if (color == 2) {
      yellow = diodBright;
    } else if (color == 1) {
      green = diodBright;
    }
    light();
  }
};
