#define DEBUG 1
#include "logging.h"

#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>
#include <RTClib.h>

#include "Display.h"
#include "ClockFace.h"

//
// The RTC Keeps the time
//
RTC_DS3231 rtc;

void setupRTC() {
  bool result = rtc.begin();
  DCHECK(result, "RTC didn't start");
  
  // To set the date an without unplugging everything and removing the battery
  // just uncomment this next line.
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (rtc.lostPower()) {
    DCHECK("This should only happens on first or if the battery is removed.");
    // This is a kludge until this can be adjusted differently.
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// For the English clock face, use EnglishClockFace instead below.
FrenchClockFace clockFace(ClockFace::LightSensorPosition::Bottom);
Display display(clockFace);


void setup() {
  setupLogging()
  setupRTC();
  display.setup();
}

void loop() {
  DateTime now = rtc.now();
  display.updateForTime(now.hour(), now.minute(), now.second());
  display.loop();
}


#if 0
// This is just a simple test to flash all the LEDS and to verify the
// orientation: Dark on the bottom, light on top, red on the left, green on the
// right. This flashes once on startup.
void setTestColors() {
  RgbColor darkRed    = RgbColor(0x8b, 0x00, 0x00);
  RgbColor lightRed   = RgbColor(0xff, 0x2c, 0x2c);
  RgbColor darkGreen  = RgbColor(0x00, 0x60, 0x00);
  RgbColor lightGreen = RgbColor(0x40, 0xff, 0x40);

  pixels.SetPixelColor(TopLeft, lightRed);
  pixels.SetPixelColor(TopRight, lightGreen);
  pixels.SetPixelColor(BottomLeft, darkRed);
  pixels.SetPixelColor(BottomRight, darkGreen);

  for (int x = 0; x <= NEOPIXEL_COLUMNS; x++) {
    for (int y = 0; y <= NEOPIXEL_ROWS; y++) {
      // starts with the color that is present.
      RgbColor originalColor = pixels.GetPixelColor(topo(x,y));
      RgbColor targetColor = RgbColor::BilinearBlend(
          lightRed, darkRed, 
          lightGreen, darkGreen,
          (float)x / (float)NEOPIXEL_COLUMNS, (float)y / (float)NEOPIXEL_ROWS);
      
      AnimEaseFunction easing = NeoEase::QuadraticIn;
      
      AnimUpdateCallback animUpdate = [=](const AnimationParam& param) {
          float progress = param.progress;
          bool reverse = progress >= .5;
          
          float halfProgress = reverse? ((progress - .5) * 2.) : progress * 2.;
          // progress will start at 0.0 and end at 1.0
          // we convert to the curve we want
          float correctedProgress = easing(halfProgress);
          RgbColor updatedColor;
          if (reverse) {
            // use the curve value to apply to the animation
             updatedColor = RgbColor::LinearBlend(
                targetColor, originalColor, correctedProgress);
          } else {
          // use the curve value to apply to the animation
             updatedColor = RgbColor::LinearBlend(
              originalColor, targetColor, correctedProgress);
          }
          pixels.SetPixelColor(topo(x,y), updatedColor);
      };
      animations.StartAnimation(topo(x,y), 200, animUpdate);
    }
  }
}
#endif



#if 0
int step = 0;

void loop() {
  switch (step) {
    case 0:
      setTestColors();
      step += 1;
      break;
    case 1:
      if (!animations.IsAnimating())
        step += 1;
      break;
    case 2:
      // If the "boot" button is pressed, move time forward. It's a crude way to
      // set the time.
      bool buttonPressed = digitalRead(0) != HIGH;
      if ((!animations.IsAnimating()) && (buttonPressed)) {
        DateTime now = rtc.now() + TimeSpan(20);
        rtc.adjust(now);
      }
      // Adjust the display to show the right time
      showtime(buttonPressed? 20 : TIME_CHANGE_ANIMATION_SPEED);
      break;
  }
  animations.UpdateAnimations();
}
#endif
