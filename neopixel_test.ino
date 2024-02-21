#include <Adafruit_NeoPixel.h>

#define PIN        6
#define NUMPIXELS  20

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
}

void loop() {
  // Set all pixels to red
  colorWipe(pixels.Color(255, 0, 0), 50); // Red color with 50ms delay
  
  // Set all pixels to green
  colorWipe(pixels.Color(0, 255, 0), 50); // Green color with 50ms delay
  
  // Set all pixels to blue
  colorWipe(pixels.Color(0, 0, 255), 50); // Blue color with 50ms delay
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(wait);
  }
}
