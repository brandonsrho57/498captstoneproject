#include <ArduinoBLE.h>

#include <Servo.h>
#define PIN        6

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


void setup() {
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
