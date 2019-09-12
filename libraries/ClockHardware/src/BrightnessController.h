#pragma once

#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>

#include "LDRReader.h"

//
// Controls the brighness of a LED strip based on the light value of a
// LDR Reader.
//
// Create this object and then call setup() to initialize it and then invoke 
// loop() as often as possible.
//
class BrightnessController {
 public:
  BrightnessController(
      NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> &pixels);
  
  void setup();
  void loop();

 private:
  // The board is animating toward that value, or already reached it.
  float _target;

  // The light sensor.
  LDRReader _lightSensor;

  // The controller for brightness animations.
  NeoPixelAnimator _animations;

  // The LED strip to control.
  NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> &_pixels;  
};
