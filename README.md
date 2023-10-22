# pyramidak air firmware
DIY smart home the easy way
https://firm.jantac.net/
1) Non-limiting hardware with schema and PCB that you can simply order from https://jlcpcb.com
2) Full manual about pyramidak firmware, what all you can do with pyramidak smart switches
3) How to easily connect pyramidak firmware to open central system like Home Assistant



Big thanks to these authors who made some functions of the pyramidak firmware easier for me, 
the list of libraries that need to be imported into the Arduino IDE:

#include <DHTesp.h>
https://github.com/markruys/arduino-DHT

#include <OneWire.h>
https://github.com/PaulStoffregen/OneWire

#include <DallasTemperature.h>
https://github.com/milesburton/Arduino-Temperature-Control-Library

#include <SensirionI2CSht4x.h>
https://github.com/Sensirion/arduino-i2c-sht4x

#include <BMx280I2C.h>
https://bitbucket.org/christandlg/bmx280mi/src/master/

#include <SparkFun_VEML7700_Arduino_Library.h>
https://github.com/sparkfun/SparkFun_VEML7700_Arduino_Library

#include <paulvha_SCD30.h>
https://github.com/paulvha/scd30_on_raspberry

#include <sps30.h>
https://github.com/paulvha/sps30

#include "SGP30.h"
https://github.com/RobTillaart/SGP30

#include "DFRobot_BME680_I2C.h"
https://github.com/DFRobot/DFRobot_BME680