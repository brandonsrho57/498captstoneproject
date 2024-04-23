#include <Servo.h>
#include "filter_lib.h"
#include "FastLED.h" 
#include <Adafruit_NeoPixel.h>
#define REDPIN 8
#define GREENPIN 7
#define BLUEPIN 6
#define WhitePIN 5
#define NUM_LEDS  20
#define LED_TYPE RGBW    // Set LED type to RGBW

CRGB leds[NUM_LEDS]; 

lowpass_filter lowpassFilter(60);  //set the cuttoff frequency for the filter 
//Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
float raw_signal;
float filtered_signal;
char one = "one";
char two = "two";
char three = "three" ;
char four = "four" ;

//NeoPixel.setPixelColor(pixel, NeoPixel.Color(255, 255, 255)); 

// Set the initial (i.e., starting) state
int twinkleSpeed = 1 / (map(filtered_signal, 0, 1023, 0, 1/2000));
StatesLED state = StatesLED::one;

void nextStateLED() {

  if (state == StatesLED::one) state = StatesLED::two;
  else if (state == StatesLED::two) state = StatesLED::three;
  else if (state == StatesLED::three) state = StatesLED::four;
  else state = StatesLED::one;
}
//input string from voice rec


void setup() {
  // Setup your Arduino here
  FastLED.addLeds<LED_TYPE, DATA_PIN_RED, RGB>(leds, NUM_LEDS);    // Add the RGBW strip with separate data pins
  FastLED.addLeds<LED_TYPE, DATA_PIN_GREEN, RGB>(leds, NUM_LEDS);  // Change RGB to RGBW if needed
  FastLED.addLeds<LED_TYPE, DATA_PIN_BLUE, RGB>(leds, NUM_LEDS);   // Change RGB to RGBW if needed
  FastLED.addLeds<LED_TYPE, DATA_PIN_WHITE, RGB>(leds, NUM_LEDS);  // Change RGB to RGBW if needed
  Serial.begin(115200);
  pixels.begin();
}

void loop() {
int raw_signal = analogRead(A0);

float t = micros() / 1e6;
Serial.println(raw_signal);
filtered_signal = lowpassFilter.filter(raw_signal);
int pixelValue = map(filtered_signal, 0, 1023, 0, 255); // need algorithm to determin hue from this
Serial.println(filtered_signal);

 
  switch(state) {
    case StatesLED::one:
    //simple on
      fill_solid(leds, NUM_LEDS, CRGBW::White);
      FastLED.show(255); // Set brightness to maximum (255)
      break;
    case StatesLED::two:
    //white and then change brightness with sensor value
      fill_solid(leds, NUM_LEDS, CRGBW::Pink);
      FastLED.show(pixelValue);
      break;
    case StatesLED::three:
    //keep a steady brightness and change color with sensor value
      fill_solid(leds, NUM_LEDS, CRGB(255, 128, 0));
      FastLED.show(255);
      break;
    case StatesLED::four:
    //twinkle with speed from sensor value
      Twinkle();
     break;
    
  }
void Twinkle(){
fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  // Randomly twinkle a few pixels
  for (int i = 0; i < 5; i++) {
    int pixel = random(NUM_LEDS);
    leds[pixel] = CRGB::White; // Set the pixel to white or any desired color
  }
  
  FastLED.show();
  delay(100); // Adjust the delay for the twinkling speed
  
}
delay(200);
}
