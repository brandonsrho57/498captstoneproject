#include <Servo.h>
#include "filter_lib.h"
#include <Adafruit_NeoPixel.h>
#define PIN        6
#define NUMPIXELS  20
Servo servo1;

lowpass_filter lowpassFilter(60);  //set the cuttoff frequency for the filter 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
float raw_signal;
float filtered_signal;
//int servo1pos = 0;

enum StatesServo {
  OPEN,
  MIDDLE,
  CLOSED
};

enum StatesLED {
  one,
  two,
  three,
  four
};

// Set the initial (i.e., starting) state
StatesServo state = StatesServo::OPEN;
//StatesLED state = StatesLED::one;

void nextStateServo() {
  servo1.attach(9); 
  if (state == StatesServo::OPEN) state = StatesServo::MIDDLE;
  else if (state == StatesServo::MIDDLE) state = StatesServo::CLOSED;
  else state = StatesServo::OPEN;
}

//void nextStateLED() {
  //servo1.attach(9); 
//  if (state == StatesLED::one) state = StatesLED::two;
  //else if (state == StatesLED::two) state = StatesLED::three;
//  else if (state == StatesLED::three) state = StatesLED::four;
  //else state = States::one;
//}


// would be cool to be able to set threshold based on a setup flex
void setup() {
  // Setup your Arduino here
  Serial.begin(115200);
  pixels.begin();
}

void loop() {
int raw_signal = analogRead(A0);

float t = micros() / 1e6;
Serial.println(raw_signal);
filtered_signal = lowpassFilter.filter(raw_signal);
int pixelColor = map(filtered_signal, 0, 1023, 0, 255); // need algorithm to determin hue from this
Serial.println(filtered_signal);

  // State-aware actions
  switch(state) {
    case StatesServo::OPEN:
      servo1.write(5);
      break;
    case StatesServo::MIDDLE:
      servo1.write(90);
      break;
    case StatesServo::CLOSED:
      servo1.write(175);
      break;
  }
 
//filtered_signal > 500
if (filtered_signal > 200){
  int startTime = millis();
  if (filtered_signal > 500 && (millis - startTime  >  2000));
   nextStateServo();
  }
delay(200);
}
