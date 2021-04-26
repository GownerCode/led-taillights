#include <Arduino.h>
#define FASTLED_INTERNAL
#include <FastLED.h>
#include <LinkedList.h>

void sinaloid_init(CRGB leds[], int NUM_LEDS, int brightness, int color)
// Sweeps a sine wave across the display to turn it on
{
    int bpm = 28;
    int pos1 = 0;
    int pos2 = 0;
    bool fade = 1;
    bool reached_end = 0;
    while (pos1 != (NUM_LEDS / 2) - 1)
    {
        pos1 = beatsin16(bpm, 0, NUM_LEDS - 1);
        pos2 = map(pos1, 0, NUM_LEDS - 1, NUM_LEDS - 1, 0);

        leds[pos1] = CHSV(color, 255, brightness);
        leds[pos2] = CHSV(color, 255, brightness);
        if (fade)
        {
            fadeToBlackBy(leds, NUM_LEDS, 15);
        }
        if (pos1 == NUM_LEDS - 1)
        {
            reached_end = 1;
        }
        if (pos1 == NUM_LEDS - 2 && reached_end)
        {
            fade = 0;
        }
        FastLED.show();
        FastLED.delay(5);
    }
}

void printList(String s, int a[], int l)
// Turns on one LED after another in a random order and increasing speed
{
    Serial.print(s);
    Serial.print(" ");
    for (int i = 0; i < l; i++)
    {
        Serial.print(a[i]);
        Serial.print(" ");
    }
    Serial.println("");
}

void ftg_init(CRGB leds[], int NUM_LEDS, int brightness, int color)
{
    LinkedList<int> indices = LinkedList<int>();
    int delay = 35;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        indices.add(i);
    }
    for (int i = 0; i < NUM_LEDS; i++)
    {
        int r = random8(indices.size() - 1);
        leds[indices.remove(r)] = CHSV(color, 255, brightness);
        FastLED.delay(delay -= (delay / NUM_LEDS));
    }
}