/*

Word clock project test sketch

This is a debug program for the light sensor.

Open the serial monitor (green looking glass icon at the right corner of the 
toolbar) and set the speed to 115200 bauds. Press the Send button.

If everything works you will see the the value measured for the sensor, ranging
from 0 to 4095 depending on how much light the LDR receives. Place your palm
above the sensor to reduce the light it receives and observe how the value 
changes in the serial monitor.

*/

#define LDR_PIN 33

int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("Word Clock LDR test program");
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(LDR_PIN);
  Serial.println(sensorValue);
  delay(100);
}
