#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>
#include <RTClib.h>

#define DCHECK

// Set the orientation of the board.
#define LIGHT_SENSOR_ON_TOP 1

RTC_DS3231 rtc;

void setupRTC() {
  bool result = rtc.begin();
  DCHECK(result);

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

// Defines how the matrix is laid out, and how to access the pixels.
#define NEOPIXEL_PIN       32
#define NEOPIXEL_SIGNALS    4
#define NEOPIXEL_ROWS      11
#define NEOPIXEL_COLUMNS   10
#define NEOPIXEL_COUNT     (NEOPIXEL_ROWS * NEOPIXEL_COLUMNS + NEOPIXEL_SIGNALS)

// The main object used to access the pixel matrix.
NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod>
    pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN);

#if defined(LIGHT_SENSOR_ON_TOP)
#define PIXEL_LAYOUT ColumnMajorAlternating90Layout
#else
#define PIXEL_LAYOUT ColumnMajorAlternating270Layout
#endif  // LIGHT_SENSOR_ON_TOP

NeoTopology<PIXEL_LAYOUT>
  pixelsTopology(NEOPIXEL_ROWS, NEOPIXEL_COLUMNS);

// Returns the index of the desired pixel in the matrix. (0,0) is top left.
#define topo(x,y) (pixelsTopology.Map(x,y) + NEOPIXEL_SIGNALS)

// The indices of the additional corner lights.
enum Signals {
#if defined(LIGHT_SENSOR_ON_TOP)
  TopLeft = 0,
  BottomLeft,
  BottomRight,
  TopRight
#else
  BottomRight = 0,
  TopRight,
  TopLeft,
  BottomLeft
#endif  //LIGHT_SENSOR_ON_TOP
};

// Animation time management object.
NeoPixelAnimator animations(NEOPIXEL_COUNT, NEO_CENTISECONDS);


/* Custom French face. Can show time with some less usual variations like
   MINUIT TROIS QUARTS or DEUX HEURES PILE. This also includes all the letters
   of the alphabets as well as ? and !. Letters in lowercase below are not used
   by the clock.

  ILbESTjDEUX
  QUATRETROIS
  NEUFUNESEPT
  HUITSIXCINQ
  ONZEwHEURES
  MIDIkMINUIT
  MOINSyLEDIX
  ETTROISDEMI
  VINGT-CINQ?
  QUARTSPILE!
*/

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
#define H_ONZE       0,4, 4
                    
#define H_HEURE      5,4, 5
#define H_HEURES     5,4, 6
                    
#define H_MIDI       0,5, 4
#define H_MINUIT     5,5, 6
                    
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


bool updateState[NEOPIXEL_COUNT];

void updateSegment(int x, int y, int length) {
  for (int i = x; i <= x + length - 1; i++)
    updateState[topo(i,y)] = true;
}

void timeDecoder(int hours, int minutes) {
  int leftover = minutes % 5;
  minutes = minutes - leftover;

  if (minutes >= 35)
    hours += 1;  // Switch to "TO" minutes the next hour
  
  updateSegment(S_IL);
  updateSegment(S_EST);

  switch (hours) {
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
  }
  switch (hours) {
    case  0: case 12:
      break;
    case  1: case 13:
      updateSegment(H_HEURE); break;
    default:
      updateSegment(H_HEURES); break;
  }

  switch (minutes) {
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
      updateSegment(10, 9, 1);
  }
  
  switch (leftover) {
    case 4:
      updateState[TopLeft] = true;
    case 3:
      updateState[BottomLeft] = true;
    case 2:
      updateState[BottomRight] = true;
    case 1:
      updateState[TopRight] = true;
    case 0:
      break;
  }
}

#define TIME_CHANGE_ANIMATION_SPEED 400

int previousHour = -1;
int previousMinute = -1;

void showtime() {
  // Live in the future as the time need to be correct by the time the animation
  // ends.
  DateTime now = rtc.now() + TimeSpan(TIME_CHANGE_ANIMATION_SPEED / 100);

  if ((now.hour() == previousHour) && (now.minute() == previousMinute)) {
    return;  // Nothing to update.
  }
  previousHour = now.hour();
  previousMinute = now.minute();

  for (int i = 0; i < NEOPIXEL_COUNT; i++)
    updateState[i] = false;

  timeDecoder(now.hour(), now.minute());
  
  RgbColor white = RgbColor(0xff, 0xff, 0xff);
  RgbColor black = RgbColor(0x00, 0x00, 0x00);
  RgbColor blue = RgbColor(0x10, 0x10, 0xFF);

  for (int index = 0; index < NEOPIXEL_COUNT; index++) {
    RgbColor originalColor = pixels.GetPixelColor(index);
    RgbColor targetColor = updateState[index] ? white : black;
        
    AnimUpdateCallback animUpdate =
      [=](const AnimationParam& param) {
        float progress = NeoEase::QuadraticIn(param.progress);

        RgbColor updatedColor = RgbColor::LinearBlend(
              originalColor, targetColor, progress);
        pixels.SetPixelColor(index, updatedColor);
    };
    animations.StartAnimation(index, TIME_CHANGE_ANIMATION_SPEED, animUpdate);
  }
}


void setup() {
  setupRTC();
  pixels.Begin();
  pixels.SetBrightness(128);
}

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
    case 2:
      showtime();
      break;
  }
  animations.UpdateAnimations();
  pixels.Show();
}