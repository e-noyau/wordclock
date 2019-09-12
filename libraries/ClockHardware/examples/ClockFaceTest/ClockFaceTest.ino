#define DEBUG 1
#include <logging.h>

#include <ClockFace.h>


void setup() {
  setupLogging()
};


void loop() {
  bool result;
  int count;

  FrenchClockFace face(ClockFace::LightSensorPosition::Bottom);
  const std::vector<bool>& state = face.getState();
 
  result = face.stateForTime(12, 00, 00);
  DCHECK(result, "Failed 12:00!");
  // This expects "Il est midi", aka 9 characters lit.
  count = std::count(state.begin(), state.end(), true);
  DCHECK(count == 9, count);

  result = face.stateForTime(12, 01, 00);
  DCHECK(result, "Failed 12:01!");
  // This expects "Il est midi" and one dot, aka 10 characters lit.
  count = std::count(state.begin(), state.end(), true);
  DCHECK(count == 10, count);

  delay(2000);
};
