#include <Arduino.h>
#define FASTLED_INTERNAL

#include <FastLED.h>


#define NUM_LEDS 44
#define LED_TYPE WS2811
#define COLOR_ORDER GRB

#define DATA_PIN 3

#define left_blinker_pin 8
#define brake_pin 9
#define right_blinker_pin 10
#define reverse_pin 7

#define RATIO_BLINKER 2.5
#define BLINKER_COLOR HUE_ORANGE//20
#define blinker_brightness 255
#define blinker_fade 5
#define blinker_speed 0.3
#define hazard_duration 100


#define BRAKE_COLOR HUE_RED
#define brake_brightness 100
#define backlight_brightness 100
#define reverse_brightness brake_brightness + backlight_brightness

#define idle_brightness 100

#define FPS 120

#define LEFT 0
#define RIGHT 1
#define BOTH 2
#define NONE -1


CRGB leds[NUM_LEDS];

bool braking = 1;
float blinker_pos = 43;
int blinking = NONE;
int hazard_timer = 0;
bool reversing = false;

void initialize(int hs){
  CHSV leds_aux[NUM_LEDS];
  if(hs>0){
    
  
  for(int i = 0; i<NUM_LEDS; i++){
    leds_aux[i]= CHSV(0,0,0);
  }}
  int fade = 20;
  int hue = BRAKE_COLOR;
  int huestep = hs;
  int up = (NUM_LEDS/2)+1;
  int down = (NUM_LEDS/2)-1;
  leds_aux[NUM_LEDS/2] = CHSV(hue, 255, backlight_brightness+brake_brightness);
  while(up<NUM_LEDS-1 || down>0){
    leds_aux[up] = CHSV(hue+=huestep, 255, backlight_brightness+brake_brightness);
    leds_aux[down] = CHSV(hue+=huestep, 255, backlight_brightness+brake_brightness);
    up++;
    down--;
    for(int i = 0; i<NUM_LEDS; i++){
      if(leds_aux[i].v > backlight_brightness){
        leds_aux[i].v  -= fade; 
      }
    }

    for(int i=0;i<NUM_LEDS;i++){
      leds[i] = leds_aux[i];
    }
    
    FastLED.show();
    FastLED.delay(1000/50);
    
  }
  while(leds_aux[0].v>backlight_brightness){
    for(int i = 0; i<NUM_LEDS; i++){
      if(leds_aux[i].v > backlight_brightness){
        leds_aux[i].v  -= fade; 
      }
    }

    for(int i=0;i<NUM_LEDS;i++){
      leds[i] = leds_aux[i];
    }
    
    FastLED.show();
    FastLED.delay(1000/1);
  }
  
}


void setup() {
  pinMode(left_blinker_pin, INPUT);
  pinMode(brake_pin, INPUT);
  pinMode(right_blinker_pin, INPUT);
  pinMode(reverse_pin, INPUT);

  delay(500);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(idle_brightness);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  initialize(255/(NUM_LEDS/2));
  initialize(0);

}

void blinker(int dir)
{
  if(dir == RIGHT){
    int start = NUM_LEDS-(NUM_LEDS/RATIO_BLINKER);
    int fin = NUM_LEDS-1;
    blinker_pos+=blinker_speed;    
    leds[(int)blinker_pos] = CHSV(BLINKER_COLOR, 255, blinker_brightness);
    if(blinker_pos < start || blinker_pos > fin) blinker_pos = start;
  }
  else if(dir == LEFT){
    int start = (NUM_LEDS/RATIO_BLINKER);
    int fin = 0;
    blinker_pos -= blinker_speed;
    if(blinker_pos > start || blinker_pos < fin) blinker_pos = start;
    
    leds[(int)blinker_pos] = CHSV(BLINKER_COLOR, 255, blinker_brightness);
  }
  else if(dir == BOTH){
    if(hazard_timer == 0){
      fill_solid(leds, NUM_LEDS, CRGB::Orange);
      hazard_timer = hazard_duration;
    }
    hazard_timer--;
  }
  else{
    
  }
  fadeToBlackBy(leds, NUM_LEDS, blinker_fade);
}

void update_strip(){

  if(reversing){
    fill_solid(leds, NUM_LEDS, CHSV(0, 0, reverse_brightness));
    return;

  }
  
  if(blinking == RIGHT){
    for(int i=0; i<(NUM_LEDS-(NUM_LEDS/RATIO_BLINKER)); i++){
      leds[i]=CHSV(BRAKE_COLOR, 255, 100+braking*brake_brightness);
    }
  }
  else if(blinking == LEFT){
    for(int i=NUM_LEDS/RATIO_BLINKER; i<NUM_LEDS; i++){
      leds[i]=CHSV(BRAKE_COLOR, 255, 100+braking*brake_brightness);
    }
  }
  else if(blinking== BOTH){
    for(int i=NUM_LEDS/RATIO_BLINKER; i<(NUM_LEDS-(NUM_LEDS/RATIO_BLINKER)); i++){
      leds[i]=CHSV(BRAKE_COLOR, 255, 100+braking*brake_brightness);
    }
  }
  else{
    fill_solid(leds, NUM_LEDS, CHSV(BRAKE_COLOR, 255, backlight_brightness+braking*brake_brightness));
  }
}

void update_states(){
  if(digitalRead(right_blinker_pin) == 1 && digitalRead(left_blinker_pin) == 1){
    blinking = BOTH;
  }
  
  else if(digitalRead(right_blinker_pin) == 1){
    blinking = RIGHT;
  }
  else if(digitalRead(left_blinker_pin) == 1){
    blinking = LEFT;
  }
  else{
    blinking = NONE;
    digitalWrite(13, LOW);
  }
  reversing = digitalRead(reverse_pin);
  braking = digitalRead(brake_pin);
}


void loop() {
  update_states();
  blinker(blinking);
  update_strip();
  
  FastLED.show();
  FastLED.delay(1000/FPS);
}