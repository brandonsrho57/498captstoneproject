#include <ArduinoBLE.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include "filter_lib.h"
#define PIN     6 // Example pin number for the data input on the LED strip
#define NUM_PIXELS    60 // Number of LEDs in your strip
int PixelValue;
Servo servo1;

//StatesServo state = StatesServo::OPEN;
      
enum {
  GESTURE_NONE  = -1,
  GESTURE_UP    = 0,
  GESTURE_DOWN  = 1,
  GESTURE_LEFT  = 2,
  GESTURE_RIGHT = 3
};

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a0618";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a0618";

int curMyo = -1;
int onOffState = 0;
int motorcommand = -1; 

BLEService wingService(deviceServiceUuid); 
BLEByteCharacteristic wingCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLEWrite);



Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
enum Commands {UNKNOWN, ONE, TWO, THREE, FOUR}; // Define enum for commands

float t = micros() / 1e6;



void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
  while (!Serial);  
  
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
  digitalWrite(LED_BUILTIN, LOW);

  
  if (!BLE.begin()) {
    Serial.println("- Starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  BLE.setLocalName("Arduino Nano 33 BLE (Peripheral)");
  BLE.setAdvertisedService(wingService);
  wingService.addCharacteristic(wingCharacteristic);
  BLE.addService(wingService);
  wingCharacteristic.writeValue(-1);
  BLE.advertise();

  Serial.println("Nano 33 BLE (Peripheral Device)");
  Serial.println(" ");

 
  servo1.attach(9);
}

void loop() {
  BLEDevice central = BLE.central();
  Serial.println("- Discovering central device...");
  delay(500);

  if (central) {
    Serial.println("* Connected to central device!");
    Serial.print("* Device MAC address: ");
    Serial.println(central.address());
    Serial.println(" ");

    while (central.connected()) {
      if (wingCharacteristic.written()) {
         //curMyo = wingCharacteristic.value();
         motorcommand = wingCharacteristic.value();
         //writeMotor(curMyo);
         motoraction(motorcommand);
       }
    }
    
    Serial.println("* Disconnected to central device!");
  }



  
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

void motoraction(int command){
  // State-aware actions
  switch(command) {
    case 1:
      servo1.write(5);
      break;
    case 0:
      servo1.write(90);
      break;
    case -1:
      servo1.write(175);
      break;
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
  //delay(twinkleSpeed); // Delay to control the speed of the twinkle
  strip.setPixelColor(pixel, 0); // Turn off the pixel
  strip.show(); // Update the LEDs
}

void writeMotor(int curMyo) {
  Serial.println("Motor status has changed!");

  if (curMyo == 1 && onOffState == 0){
    onOffState = 1;
  }else if (curMyo == 1 && onOffState == 1){
    onOffState = 0;
  }
  
   switch (onOffState) {
      case 0:
        Serial.println("* Actual value: OFF (red LED on)");
        Serial.println(" ");
        digitalWrite(LEDR, LOW);
        digitalWrite(LEDG, HIGH);
        digitalWrite(LEDB, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        break;
      case 1:
        Serial.println("* Actual value: ON (green LED on)");
        Serial.println(" ");
        digitalWrite(LEDR, HIGH);
        digitalWrite(LEDG, LOW);
        digitalWrite(LEDB, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        break;
      default:
        digitalWrite(LEDR, HIGH);
        digitalWrite(LEDG, HIGH);
        digitalWrite(LEDB, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        break;
    }      
}
