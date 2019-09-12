/* 

Word clock project test sketch

Date and time functions using a DS3231 RTC connected via I2C.

Open the serial monitor (green looking glass icon at the right corner of the 
toolbar) and set the speed to 115200 bauds. Press the Send button.

If everything works you will see the time and date printed in the monitor every 
3 seconds. You can unplug the ESP32, wait for a little, close the serial monitor
and reopen it, then plug the ESP32 back and the time should start printing in 
the monitor again, showing that the backup battery works.

*/
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

void setup () {

  Serial.begin(115200);

  delay(3000); // wait for console opening
   
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Comment this out to force the time to be set to last compile date.
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void loop () {
    DateTime now = rtc.now();
    char buf[] = "Today is DDD, MMM DD YYYY at hh:mm:ss";
    Serial.println(now.toString(buf));
    
    delay(3000);
}
