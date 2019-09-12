#define DEBUG 1
#include "logging.h"

#include <NeoPixelBus.h>  // Only need NeoTopology

#include "ClockFace.h"

// The number of LEDs connected before the start of the matrix.
#define NEOPIXEL_SIGNALS    4

// Matrix dimensions.
#define NEOPIXEL_ROWS      11
#define NEOPIXEL_COLUMNS   10

// Number of LED on the whole strip
#define NEOPIXEL_COUNT (NEOPIXEL_ROWS * NEOPIXEL_COLUMNS + NEOPIXEL_SIGNALS)

// static
int ClockFace::pixelCount() {
  return NEOPIXEL_COUNT;
}

ClockFace::ClockFace(LightSensorPosition position) : 
    _hour(-1), _minute(-1), _second(-1),
    _position(position), _state(NEOPIXEL_COUNT) {};


void ClockFace::setLightSensorPosition(LightSensorPosition position) {
  _position = position;
}

uint16_t ClockFace::map(int16_t x, int16_t y) {
  switch(_position) {
    case LightSensorPosition::Top: {
      static NeoTopology<ColumnMajorAlternating90Layout> sensor_on_top(
          NEOPIXEL_ROWS, NEOPIXEL_COLUMNS);
      return sensor_on_top.Map(x, y) + NEOPIXEL_SIGNALS;
    }
    case LightSensorPosition::Bottom: {
      static NeoTopology<ColumnMajorAlternating270Layout> sensor_on_bottom(
          NEOPIXEL_ROWS, NEOPIXEL_COLUMNS);
      return sensor_on_bottom.Map(x, y) + NEOPIXEL_SIGNALS;
    }
    default:
      DCHECK(false, static_cast<int>(_position));
  }
}

uint16_t ClockFace::mapMinute(Corners corner) {
  switch(_position) {
    case LightSensorPosition::Bottom:
      return (static_cast<uint16_t>(corner) + 2) % 4;
    case LightSensorPosition::Top:
      return static_cast<uint16_t>(corner);
    default:
      DCHECK(false, static_cast<int>(corner));
  }
}

// Lit a segment in updateState.
void ClockFace::updateSegment(int x, int y, int length) {
  for (int i = x; i <= x + length - 1; i++)
    _state[map(i,y)] = true;
}


//
// Constants to match the ClockFace.
//
// Custom French face. Can show time with some less usual variations like
// MINUIT TROIS QUARTS or DEUX HEURES PILE. This also includes all the letters
// of the alphabets as well as !. Letters in lowercase below are not used
// by the clock.

// ILbESTjDEUX
// QUATRETROIS
// NEUFUNESEPT
// HUITSIXCINQ
// MIDIXMINUIT
// ONZEwHEURES
// MOINSyLEDIX
// ETTROISDEMI
// VINGT-CINQk
// QUARTSPILE!
//

// All the segments of words on the board. The first too numbers are the
// coordinate of the first letter of the word, the last is the length. A word
// must always be on one row.
#define S_IL         0,0, 2
#define S_EST        3,0, 3

#define H_UNE        4,2, 3
#define H_DEUX       7,0, 4
#define H_TROIS      6,1, 5
#define H_QUATRE     0,1, 6
#define H_CINQ       7,3, 4
#define H_SIX        4,3, 3
#define H_SEPT       7,2, 4
#define H_HUIT       0,3, 4
#define H_NEUF       0,2, 4
#define H_DIX        2,4, 3
#define H_ONZE       0,5, 4
                    
#define H_HEURE      5,5, 5
#define H_HEURES     5,5, 6
                    
#define H_MIDI       0,4, 4
#define H_MINUIT     5,4, 6
                    
#define M_MOINS      0,6, 5
#define M_LE         6,6, 2
#define M_ET         0,7, 2
#define M_TROIS      2,7, 5

#define M_DIX        8,6, 3
#define M_QUART      3,7, 5
#define M_VINGT      0,8, 5
#define M_VINGTCINQ  0,8,10
#define M_CINQ       6,8, 4
#define M_DEMI       7,7, 4
#define M_QUART      0,9, 5
#define M_QUARTS     0,9, 6
#define M_PILE       6,9, 4

bool FrenchClockFace::stateForTime(int hour, int minute, int second) {
  if (hour == _hour && minute == _minute) {
    return false;
  }
  _hour = hour; _minute = minute;

  DLOGLN("update state");

  // TODO move to a more convenient place
  // Reset the board to all black
  for (int i = 0; i < NEOPIXEL_COUNT; i++)
    _state[i] = false;

  int leftover = minute % 5;
  minute = minute - leftover;

  if (minute >= 35)
    hour = (hour + 1) % 24;  // Switch to "TO" minutes the next hour
  
  updateSegment(S_IL);
  updateSegment(S_EST);

  switch (hour) {
    case  0:
      updateSegment(H_MINUIT); break;
    case  1: case 13:
      updateSegment(H_UNE); break;
    case  2: case 14:
      updateSegment(H_DEUX); break;
    case  3: case 15:
      updateSegment(H_TROIS); break;
    case  4: case 16:
      updateSegment(H_QUATRE); break;
    case  5: case 17:
      updateSegment(H_CINQ); break;
    case  6: case 18:
      updateSegment(H_SIX); break;
    case  7: case 19:
      updateSegment(H_SEPT); break;
    case  8: case 20:
      updateSegment(H_HUIT); break;
    case  9: case 21:
      updateSegment(H_NEUF); break;
    case 10: case 22:
      updateSegment(H_DIX); break;
    case 11: case 23:
      updateSegment(H_ONZE); break;
    case 12: 
      updateSegment(H_MIDI); break;
    default:
      DLOG("Invalid hour ");
      DLOGLN(hour);
  }
  switch (hour) {
    case  0: case 12:
      break;
    case  1: case 13:
      updateSegment(H_HEURE); break;
    default:
      updateSegment(H_HEURES); break;
  }

  switch (minute) {
    case 0:
      break;
    case 5:
      updateSegment(M_CINQ); break;
    case 10:
      updateSegment(M_DIX); break;
    case 15:
      updateSegment(M_ET); updateSegment(M_QUART); break;
    case 20:
      updateSegment(M_VINGT); break;
    case 25:
      updateSegment(M_VINGTCINQ); break;
    case 30:
      updateSegment(M_ET); updateSegment(M_DEMI); break;
    case 35:
      updateSegment(M_MOINS); updateSegment(M_VINGTCINQ); break;
    case 40:
      updateSegment(M_MOINS); updateSegment(M_VINGT); break;
    case 45:
      updateSegment(M_MOINS); updateSegment(M_LE); updateSegment(M_QUART); break;
    case 50:
      updateSegment(M_MOINS); updateSegment(M_DIX); break;
    case 55:
      updateSegment(M_MOINS); updateSegment(M_CINQ); break;
    default:
      DLOG("Invalid minute ");
      DLOGLN(minute);
  }
  
  switch (leftover) {
    case 4:
      _state[mapMinute(TopLeft)] = true;
    case 3:  // fall through
      _state[mapMinute(BottomLeft)] = true;
    case 2:  // fall through
      _state[mapMinute(BottomRight)] = true;
    case 1:  // fall through
      _state[mapMinute(TopRight)] = true;
    case 0:  // fall through
      break;
  }
  return true;
}
