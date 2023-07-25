/*
 * Arduino interface for the use of RGB 2835 strip LEDs
 * Uses Adalight protocol and is compatible with Boblight, Prismatik, etc...
 * "Magic Word" for synchronization is 'Ada' followed by LED High, Low, and Checksum
 * This code assumes the RGB 2835 strip is connected to separate pins for R, G, B, and Power
 * Modify the pin numbers according to your setup
 * 
 * Original author: Wifsimster <wifsimster@gmail.com>
 * Modified by: Your Name
 * @library: FastLED v3.001
 * @date: 11/22/2015
 */

#include "FastLED.h"
#define NUM_LEDS 240
#define DATA_PIN_R 6 // Pin for Red
#define DATA_PIN_G 7 // Pin for Green
#define DATA_PIN_B 8 // Pin for Blue
#define DATA_PIN_POWER 9 // Pin for Power connection

// Baudrate, higher rate allows faster refresh rate and more LEDs (defined in /etc/boblight.conf)
#define serialRate 115200

// Adalight sends a "Magic Word" (defined in /etc/boblight.conf) before sending the pixel data
uint8_t prefix[] = {'A', 'd', 'a'}, hi, lo, chk, i;

// Initialize LED-array
CRGB leds[NUM_LEDS];

void setup() {
  // Use NEOPIXEL to keep true colors
  FastLED.addLeds<NEOPIXEL, DATA_PIN_R>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<NEOPIXEL, DATA_PIN_G>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<NEOPIXEL, DATA_PIN_B>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  
  // Initial RGB flash
  LEDS.showColor(CRGB(255, 0, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 255, 0));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 255));
  delay(500);
  LEDS.showColor(CRGB(0, 0, 0));
  
  Serial.begin(serialRate);
  // Send "Magic Word" string to host
  Serial.print("Ada\n");
}

void loop() { 
  // Wait for the first byte of the Magic Word
  for(i = 0; i < sizeof prefix; ++i) {
    waitLoop: while (!Serial.available()) ;;
    // Check the next byte in the Magic Word
    if(prefix[i] == Serial.read()) continue;
    // Otherwise, start over
    i = 0;
    goto waitLoop;
  }
  
  // Hi, Lo, Checksum  
  while (!Serial.available()) ;;
  hi = Serial.read();
  while (!Serial.available()) ;;
  lo = Serial.read();
  while (!Serial.available()) ;;
  chk = Serial.read();
  
  // If the checksum does not match, go back to wait
  if (chk != (hi ^ lo ^ 0x55)) {
    i = 0;
    goto waitLoop;
  }
  
  memset(leds, 0, NUM_LEDS * sizeof(struct CRGB));
  // Read the transmission data and set LED values
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    byte r, g, b;    
    while(!Serial.available());
    r = Serial.read();
    while(!Serial.available());
    g = Serial.read();
    while(!Serial.available());
    b = Serial.read();
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
  }
  
  // Shows new values
  FastLED.show();
}
