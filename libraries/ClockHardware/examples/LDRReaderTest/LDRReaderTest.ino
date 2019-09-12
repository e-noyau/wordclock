/*

Word clock project test sketch

This is a debug program for the light sensor.

Open the serial monitor (green looking glass icon at the right corner of the 
toolbar) and set the speed to 115200 bauds. Press the Send button.

If everything works you will see the the value measured for the sensor, ranging
from 0 to 1 depending on how much light the LDR receives. Place your palm
above the sensor to reduce the light it receives and observe how the value 
changes in the serial monitor.

*/


#define DEBUG 1
#include <logging.h>

#include <LDRReader.h>

LDRReader ldrReader;

void setup() {
  setupLogging();

  ldrReader.setup();
};


void loop() {
  ldrReader.loop();
  DLOGLN(ldrReader.reading());
};

