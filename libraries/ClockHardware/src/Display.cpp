#define DEBUG 1
#include "logging.h"

#include "Display.h"


Display::Display(ClockFace& clockFace, uint8_t pin)
    : _clockFace(clockFace), 
      _pixels(ClockFace::pixelCount(), pin),
      _brightnessController(_pixels),
      _animations(ClockFace::pixelCount(), NEO_CENTISECONDS) {}

void Display::setup() {
  _pixels.Begin();
  _brightnessController.setup();
}

void Display::loop() {
  // TODO This has nothing to do here, remove somewhere else.
      // // If the "boot" button is pressed, move time forward. It's a crude way to
      // // set the time.
      // bool buttonPressed = digitalRead(0) != HIGH;
      // if ((!animations.IsAnimating()) && (buttonPressed)) {
      //   DateTime now = rtc.now() + TimeSpan(20);
      //   rtc.adjust(now);
      // }
      // // Adjust the display to show the right time
      // showtime(buttonPressed? 20 : TIME_CHANGE_ANIMATION_SPEED);
      // break;
  _brightnessController.loop();

  _animations.UpdateAnimations();
  _pixels.Show();
}


void Display::updateForTime(int hour, int minute, int second, int animationSpeed) {

  if (!_clockFace.stateForTime(hour, minute, second)) {
    return;  // Nothing to update.
  }
  
  static const RgbColor white = RgbColor(0xff, 0xff, 0xff);
  static const RgbColor black = RgbColor(0x00, 0x00, 0x00);

  DLOG("Time: ");
  DLOG(hour);
  DLOG(":");
  DLOGLN(minute);

  // For all the LED animate a change from the current visible state to the new
  // one.
  const std::vector<bool>& state = _clockFace.getState();
  for (int index = 0; index < state.size(); index++) {
    RgbColor originalColor = _pixels.GetPixelColor(index);
    RgbColor targetColor = state[index] ? white : black;
        
    AnimUpdateCallback animUpdate = [=](const AnimationParam& param) {
        float progress = NeoEase::QuadraticIn(param.progress);
        RgbColor updatedColor = RgbColor::LinearBlend(
            originalColor, targetColor, progress);
        _pixels.SetPixelColor(index, updatedColor);
    };
    _animations.StartAnimation(index, animationSpeed, animUpdate);
  }
}

