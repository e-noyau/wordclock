#define DEBUG 1
#include "logging.h"

#include "BrightnessController.h"

#if not defined(DEBUG)
// Uncomment to output data suitable to use with the arduino serial plotter.
//#define PLOT_BRIGHTNESS 1
#endif

#define MIN_BRIGHTNESS 20
#define MAX_BRIGHTNESS 255

#define HALF_BRIGHTNESS ((MAX_BRIGHTNESS - MIN_BRIGHTNESS) / 2 + MIN_BRIGHTNESS)

BrightnessController::BrightnessController(
    NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> &pixels)
    : _pixels(pixels),
      _target(HALF_BRIGHTNESS), 
      _animations(1, NEO_CENTISECONDS) {
}

void BrightnessController::setup() {
  _lightSensor.setup();
  _pixels.SetBrightness((uint8_t)_target);
}

void BrightnessController::loop() {
  _animations.UpdateAnimations();
  _lightSensor.loop();

  float range = MAX_BRIGHTNESS - MIN_BRIGHTNESS;
  float corrected = (range * _lightSensor.reading()) + MIN_BRIGHTNESS;

  if (abs(_target - corrected) < 5.) {
    return;  // don't adjust for small changes.
  }
  // Store the target value as an animation is about to start
  _target = corrected;

  // The starting state is the current brightness value on the board.
  float currentBrightness = (float)_pixels.GetBrightness();

  DLOG("Brightness animating from ");
  DLOG(currentBrightness, 1);
  DLOG(" to ");
  DLOGLN(corrected, 1);

  AnimUpdateCallback animUpdate = [=](const AnimationParam& param) {
    float progress = NeoEase::QuadraticIn(param.progress);
    float brightness = (corrected - currentBrightness) * progress
                     + currentBrightness;

    _pixels.SetBrightness((uint8_t)brightness);
#if defined(PLOT_BRIGHTNESS)
      Serial.println(brightness);
      Serial.print(" ");
#endif  // PLOT_BRIGHTNESS

#if defined(DEBUG)
    if (param.state == AnimationState_Completed) {
      DLOG("Brightness animation ended at ");
      DLOGLN(brightness, 1);
    }
#endif  // DEBUG
 };
 _animations.StartAnimation(0, 1000, animUpdate);
}
