#include <EEPROM.h> //permanent memory

class Memory {

private:
  int memAlok;
  int memUsed = 175 + 50;

  void report(String msg, bool offset = false) {
    if(offset) {Serial.println("");}
    Serial.println(msg);
  }

  int findFreeMem() {
    //check the occupied memory
    int start, end;
    String text;
    start = 0;
    for (int i = 1; i < 50 + 1; i++) { 
      int one = EEPROM.read(i);
      if (one == 255) {
        break;
      } else {
        if (start == 0) {start = i;}
        end = i;      
        text += String(char(one));    
      }
    }
    report ("memOccupied: "+ text, true);
    report ("start: " + String(start) + "; end: " + String(end));
    if (end == 50) {end = 0;}
    return end + 1;
  } 

public:
  
  Memory() {}

  void begin() {
    EEPROM.begin(memUsed);  
    report("EEPROM reading", true);
    //alokace paměti
    memAlok = EEPROM.read(0);
    if (memAlok > 50) {
      memAlok = 1;    
    } else if (memAlok == 255) {
      memAlok = findFreeMem();
    }
    report("memAlokFrom: " + String(memAlok)); 
    if (EEPROM.read(0) != memAlok) {
      clear();
      EEPROM.write(0, memAlok);
      EEPROM.commit();
    }
  }

  void clear() {
    bool change;
    for (int i = memAlok; i <= memUsed; i++) { 
      if (EEPROM.read(i) != 255) {
        EEPROM.write(i, 255);
        change = true;
      }
    }  
    EEPROM.write(0, 255);
    if(change) {EEPROM.commit();}
    report("EEPROM cleared"); 
  }

  int read(int pos) {
    return EEPROM.read(pos + memAlok);
  } 

  String read(int start, int end) {
    start = start + memAlok;
    end = end + memAlok;
    String text;
    for (int i = start; i < end + 1; i++) { 
      int one = EEPROM.read(i);
      if (one == 255) {
        break;
      } else {
        text += String(char(one));    
      }
    }
    return text;
  }

  int readAndCheck(int def, int pos, String text, int min, int max) {
    int value = read(pos);
    if (value != 255) {report(text + ": " + String(value));}
    if (value >= min and value <= max) {
      return value;
    } else {
      return def;
    }
  }

  int readAndCheck(int def, int posLow, int posHigh, String text) {
    posLow += memAlok;
    posHigh += memAlok;
    int valLow = EEPROM.read(posLow);
    if (valLow == 255) valLow = 0;
    int valHigh = EEPROM.read(posHigh);
    if (valHigh == 255) valHigh = 0;
    int value = (valHigh * 254) + valLow;
    report(text + ": " + String(value));
    if (valLow == 0 and valHigh == 0) {
      return def;
    } else {
      return value;
    }
  }

  String readAndCheck(String def, String text, int start, int end, bool password) {
    String value = read(start, end);
    if (value != "") {
      if (password == true) {
        report(text + ": *"); 
      } else {
        report(text + ": " + value); 
      }   
      return value;
    } else {
      return def;
    }   
  }

  float readAndCheck(float def, int pos, String text) {
    pos = pos + memAlok;    
    union {
      float floats;
      unsigned char bytes[4];
    } convert;
    for (int i=0; i<4; i++) {
      convert.bytes[i] = EEPROM.read(pos + i);
    }
    if (isnan(convert.floats)) {
      return def;
    } else {
      report(text + ": " + String(convert.floats));
      return convert.floats;
    }
  }

  void write(float value, int pos) {
    pos = pos + memAlok;
    union {
      float floats;
      unsigned char bytes[4];
    } convert;
    convert.floats = value;
    for (int i=0; i<4; i++) {
      if (EEPROM.read(pos + i) != convert.bytes[i]) {
        EEPROM.write(pos + i, convert.bytes[i]);
      }
    }
    EEPROM.commit(); 
  }

  void write(int value, int pos) {
    pos = pos + memAlok;
    if (EEPROM.read(pos) != value) {
      EEPROM.write(pos, value);
      EEPROM.commit();
      report("EEPROM_" + String(pos) + "(" + String(pos - memAlok) + "): " + String(value));     
    }
  }

  void write(String text, int start, int end) {
    start = start + memAlok;
    end = end + memAlok;
    for (int i = 0; i < end - start + 1; i++) { 
      byte one;
      if (i < text.length()) {
        one = byte(text.charAt(i));
      } else {
        one = 255;
      }
      if (EEPROM.read(i + start) != one) {
        EEPROM.write(i + start, one);  
      }
    }
    EEPROM.commit();
    report("EEPROM_" + String(start) + "-" + String(end) + "(" + String(start - memAlok) + "-" + String(end - memAlok) + "): " + text);   
  }

  void write(int value, int posFirst, int posSecond) {
    int first = value / 250;
    int second = value - (first * 250);
    if (EEPROM.read(posFirst + memAlok) != first) {
      EEPROM.write(posFirst + memAlok, first);
      EEPROM.commit();   
    }
    if (EEPROM.read(posSecond + memAlok) != second) {
      EEPROM.write(posSecond + memAlok, second);
      EEPROM.commit();   
    }
  }

  int intToFirst(int value) { 
    return value / 250;
  }

  int intToSecond(int value) { 
    int first = value / 250;
    return value - first;
  }

};
