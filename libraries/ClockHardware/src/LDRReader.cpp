#include "Arduino.h"

#include "LDRReader.h"
#include "logging.h"

LDRReader::LDRReader(int pinNumber, float reactionSpeed)
  : _pin(pinNumber), _reactionSpeed(reactionSpeed) {
  DCHECK(reactionSpeed <= 1.0, "Too much");
  DCHECK(reactionSpeed > 0, "Not enough");
}

void LDRReader::setup() {
  pinMode(_pin, INPUT);
  _currentLDR = analogRead(_pin);  // Initial value.
}

void LDRReader::loop() {
  _currentLDR = analogRead(_pin) * _reactionSpeed +
                _currentLDR * (1 - _reactionSpeed);

  DCHECK(_currentLDR <= 4095.0, _currentLDR);
  DCHECK(_currentLDR >= 0.0, _currentLDR);
}

float LDRReader::reading() {
  return _currentLDR / 4095.0;
}

