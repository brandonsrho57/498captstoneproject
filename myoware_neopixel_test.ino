#include <Adafruit_NeoPixel.h>

#define PIN        6
#define NUMPIXELS  20

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const int sensorPin = A0;

void setup() {
  pixels.begin();
  Serial.begin(115200);
}

void loop() {
  int sensorValue = analogRead(sensorPin);
  
  // Map the sensor value to the number of pixels for color control
  int pixelColor = map(sensorValue, 0, 1023, 0, NUMPIXELS - 1);

  // Set the color of the NeoPixel strip
  pixels.clear(); // Clear the strip
  pixels.setPixelColor(pixelColor, pixels.Color(255, 0, 0)); // Set color (here red)
  pixels.show(); // Update the strip
  
  // Print sensor value for debugging
  Serial.println(sensorValue);

  delay(100); // Adjust delay according to your needs
}
