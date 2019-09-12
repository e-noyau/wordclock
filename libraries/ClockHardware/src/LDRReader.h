#pragma once

/*
 * Light sensor. Reads the ambient light and build a representative value
 * between 0.0 and 1.0. Smooth the reading by doing a moving average.
 */
class LDRReader {
  public:
    // Reaction speed must be greater than 0 and at most 1.0. This control the
    // input smoothing, the higher the number the less smooth the data will be.
    // The pin number point to the pin the data line is soldered on. 
    LDRReader(int pinNumber = 33, float reactionSpeed = .1);

    // Must be called from the ino setup and loop.
    void setup();
    void loop();

    // Returns a value between 0. (no light) and 1. (much lights)
    float reading();

  private:
    float _currentLDR;
    float _reactionSpeed;
    int _pin;
};
