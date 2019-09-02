#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>
#include <RTClib.h>

// Defines the orientation of the board.
#define LIGHT_SENSOR_ON_TOP 1

// Comment the define of DEBUG to remove all debugging logging. If debug logging
// is off it is possible to enable various plotting data to see what's going on
// with the various sensors.
#define DEBUG 1
#if defined(DEBUG)
// Enable the blinking of the board light for debugging.
#define DTICK 1

// Same as Serial.print[ln] but shorter and removed from the code in non debug mode.
#define DLOG(...) Serial.print(__VA_ARGS__)
#define DLOGLN(...) Serial.println(__VA_ARGS__)
#define DCHECK(condition, ...) \
    if (!(condition)) {        \
      DLOG("DCHECK Line ");    \
      DLOG(__LINE__);          \
      DLOG(": (");             \
      DLOG(#condition);        \
      DLOG(") ");              \
      DLOGLN(__VA_ARGS__); }
#else
// To use the plotter to see variables, uncomment the one you want to see
// #define PLOT_LDR 1
// #define PLOT_BRIGHTNESS 1

#define DLOG(...)
#define DLOGLN(...)
#define DCHECK(condition, ...)
#endif // defined(DEBUG)

/*
 * The RTC Keeps the time
 */
RTC_DS3231 rtc;

void setupRTC() {
  bool result = rtc.begin();
  DCHECK(result, "RTC didn't start");
  /*
  if (rtc.lostPower()) {
    // This is a kludge until this can be adjusted differently.
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  */
}

/*
 * Light sensor. The light sensor controls the brightness of the LEDS, to avoid
 * permanent little changes the reads are averaged over a long period, and the
 * change of brightness is animated to not be too jarring.
 */
#define LDR_PIN 33

class LDRReader {
  public:
    LDRReader(float reactionSpeed = .1): _reactionSpeed(reactionSpeed) {
      DCHECK(reactionSpeed <= 1.0, "Too much");
      DCHECK(reactionSpeed > 0, "Not enough");

      pinMode(LDR_PIN, INPUT);
      _currentLDR = analogRead(LDR_PIN);
    }

    // Call at each loop() iteration
    void update() {
      int instantValue = analogRead(LDR_PIN);
      _currentLDR = instantValue * _reactionSpeed +
                    _currentLDR * (1 - _reactionSpeed);

      DCHECK(_currentLDR <= 4095.0, _currentLDR);
      DCHECK(_currentLDR >= 0.0, _currentLDR);
#if defined(PLOT_LDR)
      Serial.println(_currentLDR);
      Serial.print(" ");
#endif  // PLOT_LDR
    }
    // Returns a value between 0. (no light) and 1. (much lights)
    float value() {
      return _currentLDR / 4095.0;
    }
  private:
    float _currentLDR;
    float _reactionSpeed;
};


/*
 * Access to the display. To address one of the four corner use the right
 * constant (TopLeft, BottomLeft, BottomRight, TopRight), and for the matrix
 * use the topo macro, with the x/y position desired. (0,0) is top left.
 */

// -----------------
// The strip of leds
// -----------------

// With the light sensor on the bottom the panel is assumed to be wired as follow when viewed from
// the front:

// 02                                           01
//   113 112 111 110 109 108 107 106 105 104 103
//   092 093 094 095 096 097 098 099 100 101 102
//   091 090 089 088 087 086 085 084 083 082 081
//   070 071 072 073 074 075 076 077 078 079 080   
//   069 068 067 066 065 064 063 062 061 060 059
//   048 049 050 051 052 053 054 055 056 057 058
//   047 046 045 044 043 042 041 040 039 038 037 
//   026 027 028 029 030 031 032 033 034 035 036
//   025 024 023 022 021 020 019 018 017 016 015
//   004 005 006 007 008 009 010 011 012 013 014
// 03                                           00

// Defines how the matrix is laid out, and how to access the pixels.
#define NEOPIXEL_PIN       32
// The pin to control the matrix

#define NEOPIXEL_SIGNALS    4
// The number of LEDs connected before the start of the matrix.

#define NEOPIXEL_ROWS      11
#define NEOPIXEL_COLUMNS   10
// Matrix dimentions.

#define NEOPIXEL_COUNT     (NEOPIXEL_ROWS * NEOPIXEL_COLUMNS + NEOPIXEL_SIGNALS)

// The main object used to access the pixel matrix.
NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod>
    pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN);

// Defines a topology depending on orientation.
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

/*
 * Animation time management object.
 * Uses centiseconds as precision, so an animation can range from 1/100 of a
 * second to a little bit more than 10 minutes.
 */
NeoPixelAnimator animations(NEOPIXEL_COUNT, NEO_CENTISECONDS);

/*
 * Constants to match the face.
 *
 * Custom French face. Can show time with some less usual variations like
   MINUIT TROIS QUARTS or DEUX HEURES PILE. This also includes all the letters
   of the alphabets as well as !.
  
   The first table is the one in the code, with 8 available spaces. Those are filled with letters to
   have the full alphabet on the board, to eventually use as a ouija board (the missing letters are
   bjkwy, with the punctuation @!? for good measure)

   The next three are optimized to open more space by grouping words.

   The second offers the same functionality but opens 12 open spaces, plus the possibility of two
   additional three letter words (Vertically U...O, horizontally S...D).

   The third limits the use of "PILE" (which is not a huge loss) to open 2 more spots, for a total
   of 14 available spaces, and this one can spell four words of two, three and four characters
   (Vertically U...O & S...., horizontally S..D, S.., with additional two 2 letters word possible
   on line 3 and 4).

   The last one is optimized the fullest, removing the second "TROIS" and the "PILE". It uses only
   9 lines, leaving space for 11 char customization and up to two 2 letter words, and a 3 vertical.

  ILbESTjDEUX  IL.EST.DEUX  IL.EST.DEUX  IL.EST.DEUX
  QUATRETROIS  UNEUFMINUIT  UNEUFMINUIT  UNEUFMINUIT
  @kUNEUFSEPT  .SEPTROISIX  .MIDIX.HUIT  ..MIDIXHUIT
  HUITSIXCINQ  .CINQUATRE.  .CINQUATRE.  ..CINQUATRE
  MIDIXMINUIT  .MIDIX.HUIT  .SEPTROISIX  .SEPTROISIX
  ONZEwHEURES  ONZE.HEURES  ONZE.HEURES  ONZE.HEURES
  MOINSyLEDIX  MOINS...DIX  MOINS..DIX.  .MOINSETDIX
  ETTROISDEMI  LETROISDEMI  PILETROIS..  .VINGT-CINQ
  VINGT-CINQ?  VINGT-CINQ.  VINGT-CINQ.  LEDEMIQUART
  QUARTSPILE!  QUARTSPILE.  DEMIQUARTS.  ...........
*/

template <typename W> class ClockFace {
protected: 
  typedef struct { int x, y, length; W word; } Segment;

  // Lit a segment in updateState.
  void updateSegment(W word) {
    Segment segment = _segments[(int)word];
    DCHECK(word == segment.word, word); 
    for (int i = segment.x; i <= segment.x + segment.length - 1; i++)
      _updateState[topo(i,segment.y)] = true;
  }

  bool _updateState[NEOPIXEL_COUNT];

private:
  // Stores the bits of the clock that need to be turned on.
  Segment _segments[];
  
public:
  virtual void timeDecoder(int hours, int minutes) = 0;
};

// All the segments representing words on the board in order of appearance
typedef enum {
  IL, EST, DEUX,
  QUATRE, TROIS,
  UNE, NEUF, SEPT,
  HUIT, SIX, CINQ, 
  MIDI, DIX, MINUIT,
  ONZE, HEURE, HEURES, 
  MOINS, LE, M_DIX,
  ET, M_TROIS, DEMI,
  VINGT, VINGTCINQ, M_CINQ, 
  QUART, QUARTS, PILE
} FrenchWords; 



class FrenchClockFace : ClockFace<FrenchWords> {
  // The first too numbers are the coordinate of the first letter of the word, the third is the
  // length. A word must always be on one row. The last item is only used to validate the table is
  // well formed.
  ClockFace<FrenchWords>::Segment _segments[] = {
    { 0,0, 2, IL },     { 3,0, 3, EST },       { 7,0, 4, DEUX },
    { 0,1, 6, QUATRE }, { 6,1, 5, TROIS },
    { 2,2, 3, UNE },    { 3,2, 4, NEUF },      { 7,2, 4, SEPT },
    { 0,3, 4, HUIT },   { 4,3, 3, SIX },       { 7,3, 4, CINQ },
    { 0,4, 4, MIDI },   { 2,4, 3, DIX },       { 5,4, 6, MINUIT },
    { 0,5, 4, ONZE },   { 5,5, 5, HEURE },     { 5,5, 6, HEURES },
    { 0,6, 5, MOINS },  { 6,6, 2, LE },        { 8,6, 3, M_DIX },
    { 0,7, 2, ET },     { 2,7, 5, M_TROIS },   { 7,7, 4, DEMI },
    { 0,8, 5, VINGT },  { 0,8,10, VINGTCINQ }, { 6,8, 4, M_CINQ },
    { 0,9, 5, QUART },  { 0,9, 6, QUARTS },    { 6,9, 4, PILE },
  };
  // From an hour and a minute updates the updateState variable to display that
  // time on the clock.
  virtual void timeDecoder(int hours, int minutes) {
    int leftover = minutes % 5;
    minutes = minutes - leftover;

    if (minutes >= 35)
      hours += 1;  // Switch to "TO" minutes the next hour
  
    updateSegment(IL);
    updateSegment(EST);

    switch (hours) {
      case  0:
        updateSegment(MINUIT); break;
      case  1: case 13:
        updateSegment(UNE); break;
      case  2: case 14:
        updateSegment(DEUX); break;
      case  3: case 15:
        updateSegment(TROIS); break;
      case  4: case 16:
        updateSegment(QUATRE); break;
      case  5: case 17:
        updateSegment(CINQ); break;
      case  6: case 18:
        updateSegment(SIX); break;
      case  7: case 19:
        updateSegment(SEPT); break;
      case  8: case 20:
        updateSegment(HUIT); break;
      case  9: case 21:
        updateSegment(NEUF); break;
      case 10: case 22:
        updateSegment(DIX); break;
      case 11: case 23:
        updateSegment(ONZE); break;
      case 12: 
        updateSegment(MIDI); break;
      default:
        DCHECK(!hours, hours);
    }
    switch (hours) {
      case  0: case 12:
        break;
      case  1: case 13:
        updateSegment(HEURE); break;
      default:
        updateSegment(HEURES); break;
    }

    switch (minutes) {
      case 0:
        break;
      case 5:
        updateSegment(M_CINQ); break;
      case 10:
        updateSegment(M_DIX); break;
      case 15:
        updateSegment(ET); updateSegment(QUART); break;
      case 20:
        updateSegment(VINGT); break;
      case 25:
        updateSegment(VINGTCINQ); break;
      case 30:
        updateSegment(ET); updateSegment(DEMI); break;
      case 35:
        updateSegment(MOINS); updateSegment(VINGTCINQ); break;
      case 40:
        updateSegment(MOINS); updateSegment(VINGT); break;
      case 45:
        updateSegment(MOINS); updateSegment(LE); updateSegment(QUART); break;
      case 50:
        updateSegment(MOINS); updateSegment(M_DIX); break;
      case 55:
        updateSegment(MOINS); updateSegment(M_CINQ); break;
      default:
        DCHECK(!minutes, minutes);
    }
  
    switch (leftover) {
      case 4:
        _updateState[TopLeft] = true;
      case 3:
        _updateState[BottomLeft] = true;
      case 2:
        _updateState[BottomRight] = true;
      case 1:
        _updateState[TopRight] = true;
      case 0:
        break;
    }
  }

};
 


#define TIME_CHANGE_ANIMATION_SPEED 400

// To avoid refreshing all the time (and possibly interrupting animations) this
// stores the previous values of hour an minute. If they haven't changed,
// nothig is updated.
int previousHour = -1;
int previousMinute = -1;

// Starts an animation to update the clock to a new time if necessary.
void showtime(int animationSpeed) {
  // Live in the future as the time need to be correct by the time the animation
  // ends.
  DateTime now = rtc.now() + TimeSpan(TIME_CHANGE_ANIMATION_SPEED / 100);

  if ((now.hour() == previousHour) && (now.minute() == previousMinute)) {
    return;  // Shortcut, nothing to update.
  }
  previousHour = now.hour();
  previousMinute = now.minute();

  // Reset the board to all black
  for (int i = 0; i < NEOPIXEL_COUNT; i++)
    updateState[i] = false;

  // Set the right led to light up
  timeDecoder(now.hour(), now.minute());
  
  RgbColor white = RgbColor(0xff, 0xff, 0xff);
  RgbColor black = RgbColor(0x00, 0x00, 0x00);

  DLOG("Time: ");
  DLOG(now.hour());
  DLOG(":");
  DLOGLN(now.minute());

  // For all the LED animate a change from the current visible state to the new
  // one.
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
    animations.StartAnimation(index, animationSpeed, animUpdate);
  }
}

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

//
// Create this object and then call update() from the main loop(). This
// controls the brighness based on the light value.
//

class BrightnessController {
#define MIN_BRIGHTNESS 40
#define MAX_BRIGHTNESS 255
 private:
   // The board is animating toward that value, or already reached it.
  float _target;
  // The light sensor.
  LDRReader _lightSensor;
  // The controller for brightness animations.
  NeoPixelAnimator _animations;

 public:
  BrightnessController(): _target(128), _animations(1, NEO_CENTISECONDS) {
    pixels.SetBrightness((uint8_t)_target);
  }

  void update() {
    _animations.UpdateAnimations();
    _lightSensor.update();

    float range = MAX_BRIGHTNESS - MIN_BRIGHTNESS;
    float corrected = (range * _lightSensor.value()) + MIN_BRIGHTNESS;
  
    if (abs(_target - corrected) < 5.) {
      return;  // don't adjust for small changes.
    }
    // Store the target value as an animation is about to start
    _target = corrected;

    // The starting state is the current brightness value on the board.
    float currentBrightness = (float)pixels.GetBrightness();

    DLOG("Brightness animating from ");
    DLOG(currentBrightness, 1);
    DLOG(" to ");
    DLOGLN(corrected, 1);

    AnimUpdateCallback animUpdate = [=](const AnimationParam& param) {
      float progress = NeoEase::QuadraticIn(param.progress);
      float brightness = (corrected - currentBrightness) * progress
                       + currentBrightness;

      pixels.SetBrightness((uint8_t)brightness);
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
};

BrightnessController brightnessController;

#if defined(DTICK) 
void blinkTime() {
  bool secondBit = rtc.now().unixtime() & 1;
  int ledValue = _lightSensor.value() * (secondBit ? 2048 : 4095);
  ledcWrite(LEDC_CHANNEL, ledValue);
}
#endif DTICK

void setup() {
  Serial.begin(115200);
  
  setupRTC();
  pixels.Begin();
  pixels.SetBrightness(128);
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
      showtime(buttonPressed? 50 : TIME_CHANGE_ANIMATION_SPEED);
      break;
  }
  brightnessController.update();
  animations.UpdateAnimations();
  pixels.Show();

  // Tictoc
#if defined(DTICK) 
  blinkTime();
#endif

}
