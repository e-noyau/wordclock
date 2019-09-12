#pragma once

#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>

#include "BrightnessController.h"
#include "ClockFace.h"

// The pin to control the matrix
#define NEOPIXEL_PIN       32

//
#define TIME_CHANGE_ANIMATION_SPEED 400

class Display {
 public:
  Display(ClockFace& clockFace, uint8_t pin = NEOPIXEL_PIN);

  void setup();
  void loop();

  // Starts an animation to update the clock to a new time if necessary.
  void updateForTime(int hour, int minute, int second, int animationSpeed = TIME_CHANGE_ANIMATION_SPEED);

 private:

   // To know which pixels to turn on and off, one needs to know which letter
   // matches which LED, and the orientation of the display. This is the job
   // of the clockFace.
   ClockFace& _clockFace;

   // Addressable bus to control the LEDs.
   NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> _pixels;

   // Reacts to change in ambient light to adapt the power of the LEDs
   BrightnessController _brightnessController;


  //
  // Animation time management object.
  // Uses centiseconds as precision, so an animation can range from 1/100 of a
  // second to a little bit more than 10 minutes.
  //
  NeoPixelAnimator _animations;
};
