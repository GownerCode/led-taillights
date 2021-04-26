#include <Arduino.h>
#define FASTLED_INTERNAL

#include <FastLED.h>
#include "initialize.h"

// LED properties
#define NUM_LEDS 44
#define LED_TYPE WS2811
#define COLOR_ORDER GRB

#define DATA_PIN 3

// GPIO pins for signals
#define reverse_pin 7
#define left_blinker_pin 8
#define brake_pin 9
#define right_blinker_pin 10

// Master brightness in percent
#define MASTER_BRIGHTNESS 100.0

// Component brightnesses in percentage of master brightness
#define BRAKE_BRIGHTNESS 100.0
#define TAIL_BRIGHTNESS 60.0
#define REVERSE_BRIGHTNESS 100.0
#define EFFECT_BRIGHTNESS 40.0

// Blinker config
// The blinker area will take up 1/x of the strip
#define RATIO_BLINKER 2.678
// Blinker CRGB color
#define BLINKER_COLOR CRGB(255, 47, 0)
// Higher number -> steeper fade
#define blinker_fade 5
#define hazard_fade 7
#define blinker_speed 0.2
#define hazard_duration 100

// Brake hue
#define BRAKE_COLOR HUE_RED

// Speed of the animations
#define FPS 120

// Directions
#define LEFT 0
#define RIGHT 1
#define BOTH 2
#define NONE -1

// Time between blinking signals in ms - 0 for continuous signal
#define SIGNAL_TIME 500.0

//--------------------------------------------------//

// CRGB array contains LEDs
CRGB leds[NUM_LEDS];

// Various state tracking and timing variables
bool braking = false;
float blinker_pos = 43;
int blinking = NONE;
int hazard_timer = 0;
bool reversing = false;
int frames;
const int state_time = ((SIGNAL_TIME + 50.0) / 1000) * FPS;
int state_timer_1 = 0;
int state_timer_2 = 0;

// Calculates real 8-bit brightness values from percentages
int actual_tail_brightness = 255 * (TAIL_BRIGHTNESS / 100);
int actual_brake_brightness = (255 * (BRAKE_BRIGHTNESS / 100)) - actual_tail_brightness;
int actual_reverse_brightness = 255 * (REVERSE_BRIGHTNESS / 100);
int actual_effects_brightness = 255 * (EFFECT_BRIGHTNESS / 100);
int actual_master_brightness = 255 * (MASTER_BRIGHTNESS / 100);

void setup()
{
  // Uncomment follwing line for debugging
  // Serial.begin(9600);

  // Pin configuration
  pinMode(left_blinker_pin, INPUT);
  pinMode(brake_pin, INPUT);
  pinMode(right_blinker_pin, INPUT);
  pinMode(reverse_pin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // LED initialization
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(actual_master_brightness);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  // Choosing a random init animation (currently there are only 2)
  randomSeed(analogRead(A2));
  if (random(2))
  {
    ftg_init(leds, NUM_LEDS, actual_tail_brightness, BRAKE_COLOR);
  }
  else
  {
    sinaloid_init(leds, NUM_LEDS, actual_tail_brightness, BRAKE_COLOR);
  }

  FastLED.show();
}

void blinker(int dir)
// Handles Blinker animations
{
  if (dir == RIGHT)
  {
    int start = NUM_LEDS - (NUM_LEDS / RATIO_BLINKER);
    int fin = NUM_LEDS - 1;
    blinker_pos += blinker_speed;
    leds[(int)blinker_pos] = BLINKER_COLOR;
    if (blinker_pos < start || blinker_pos > fin)
      blinker_pos = start;
  }
  else if (dir == LEFT)
  {
    int start = (NUM_LEDS / RATIO_BLINKER);
    int fin = 0;
    blinker_pos -= blinker_speed;
    if (blinker_pos > start || blinker_pos < fin)
      blinker_pos = start;

    leds[(int)blinker_pos] = BLINKER_COLOR;
  }
  else if (dir == BOTH)
  {
    if (hazard_timer == 0)
    {
      fill_solid(leds, NUM_LEDS, BLINKER_COLOR);
      hazard_timer = hazard_duration;
    }
    hazard_timer--;
  }

  if (blinking == BOTH)
  {
    fadeToBlackBy(leds, NUM_LEDS, hazard_fade);
  }
  else
  {
    fadeToBlackBy(leds, NUM_LEDS, blinker_fade);
  }
}

void update_strip()
// Handles brake lights and reverse lights
{
  // Blind user when reversing
  if (reversing)
  {
    fill_solid(leds, NUM_LEDS, CHSV(0, 0, actual_reverse_brightness));
    return;
  }

  // Don't draw taillight behind blinking area when blinking
  if (blinking == RIGHT)
  {
    for (int i = 0; i < (NUM_LEDS - (NUM_LEDS / RATIO_BLINKER)); i++)
    {
      leds[i] = CHSV(BRAKE_COLOR, 255, actual_tail_brightness + braking * actual_brake_brightness);
    }
  }
  else if (blinking == LEFT)
  {
    for (int i = NUM_LEDS / RATIO_BLINKER; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(BRAKE_COLOR, 255, actual_tail_brightness + braking * actual_brake_brightness);
    }
  }
  else if (blinking == BOTH)
  {
    for (int i = NUM_LEDS / RATIO_BLINKER; i < (NUM_LEDS - (NUM_LEDS / RATIO_BLINKER)); i++)
    {
      leds[i] = CHSV(BRAKE_COLOR, 255, actual_tail_brightness + braking * actual_brake_brightness);
    }
  }
  else
  {
    fill_solid(leds, NUM_LEDS, CHSV(BRAKE_COLOR, 255, actual_tail_brightness + braking * actual_brake_brightness));
  }
}

void update_states()
// Handles input signals and state timing
{
  reversing = digitalRead(reverse_pin);
  braking = digitalRead(brake_pin);

  static bool left;
  static bool right;
  left = digitalRead(left_blinker_pin);
  right = digitalRead(right_blinker_pin);

  if (state_timer_1 > 0 && state_timer_2 > 0)
  {
    blinking = BOTH;
  }

  else if (state_timer_1 > 0)
  {
    blinking = RIGHT;
  }
  else if (state_timer_2 > 0)
  {
    blinking = LEFT;
  }

  else if (state_timer_1 <= 0 && state_timer_2 <= 0)
  {
    blinking = NONE;
  }

  if (left)
  {
    state_timer_2 = state_time;
  }
  if (right)
  {
    state_timer_1 = state_time;
  }
  if (state_timer_1 > 0)
    state_timer_1--;
  if (state_timer_2 > 0)
    state_timer_2--;
}

void loop()
{
  // Keeping track of frames for later features.
  frames++;

  update_states();
  blinker(blinking);
  update_strip();
  FastLED.delay(1000 / FPS);
}