#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include "filter_lib.h"
#define PIN     6 // Example pin number for the data input on the LED strip
#define NUM_PIXELS    60 // Number of LEDs in your strip



int filtered_signal;
lowpass_filter lowpassFilter(60);  //set the cuttoff frequency for the filter 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
enum Commands {UNKNOWN, ONE, TWO, THREE, FOUR}; // Define enum for commands
int raw_signal = analogRead(A0);
float t = micros() / 1e6;
int PixelValue = map(filtered_signal, 0, 1023, 0, 255); // need algorithm to determin hue from this
int twinkleSpeed = 1 / (map(filtered_signal, 0, 1023, 0, 1/500));


void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
}

void loop() {
  
  Serial.println(raw_signal);
  filtered_signal = lowpassFilter.filter(raw_signal);
  Serial.println(filtered_signal);
  
  while (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    Commands cmd = getCommand(command);

    switch (cmd) {
      case ONE:
        //simple on
        for (int i = 0; i < NUM_PIXELS; i++) {
          strip.setPixelColor(i, strip.Color(255, 255, 255)); // Set color to white
          strip.setBrightness(200); // Set brightness (a value from 0 to 255)
         }
         // Update all pixels to reflect the changes
       strip.show();
       break;
      case TWO:
        //white and then change brightness with sensor value
        for (int i = 0; i < NUM_PIXELS; i++) {
          strip.setPixelColor(i, strip.Color(255, 255, 255)); // Set color to white
          strip.setBrightness(PixelValue); // Set brightness (a value from 0 to 255) from myo
         }
         // Update all pixels to reflect the changes
       strip.show();
       break;
      case THREE:
        for (int i = 0; i < NUM_PIXELS; i++) {
          strip.setPixelColor(i, strip.Color(255, 255, 255)); // need to figuire out how to do this varying 
          strip.setBrightness(200); // Set brightness (a value from 0 to 255)
         }
         // Update all pixels to reflect the changes
       strip.show();
      case FOUR:
        //twinkle with speed from sensor value
        strip.setBrightness(255); // a value from 0 to 255
        twinkle();
        break;
      default:
        // Optional: Handle unknown command
      strip.clear();
        break;
    }
  }
}

Commands getCommand(String command) {
  if (command == "one") return ONE;
  if (command == "two") return TWO;
  if (command == "three") return THREE;
  if (command == "four") return FOUR;
  return UNKNOWN; // Default case if no match
}

void twinkle() {
  int pixel = random(NUM_PIXELS); // Choose a random pixel
  uint32_t color = strip.Color(random(256), random(256), random(256)); // Random color
  strip.setPixelColor(pixel, color); // Turn on the pixel with the random color
  strip.show(); // Update the LEDs
  delay(twinkleSpeed); // Delay to control the speed of the twinkle
  strip.setPixelColor(pixel, 0); // Turn off the pixel
  strip.show(); // Update the LEDs
}
