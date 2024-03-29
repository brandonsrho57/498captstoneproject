/*
  BLE_Central_Device.ino

  This program uses the ArduinoBLE library to set-up an Arduino Nano 33 BLE Sense 
  as a central device and looks for a specified service and characteristic in a 
  peripheral device. If the specified service and characteristic is found in a 
  peripheral device, the last detected value of the on-board gesture sensor of 
  the Nano 33 BLE Sense, the APDS9960, is written in the specified characteristic. 

  The circuit:
  - Arduino Nano 33 BLE Sense. 

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <Arduino_APDS9960.h>

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a0618";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a0618";

int gesture = -1;
int oldGestureValue = -1;   
int cumMyo[20] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int motorCommand = -1;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  if (!APDS.begin()) {
    Serial.println("* Error initializing APDS9960 sensor!");
  } 

  APDS.setGestureSensitivity(80); 
  
  if (!BLE.begin()) {
    Serial.println("* Starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  
  BLE.setLocalName("Nano 33 BLE (Central)"); 
  BLE.advertise();

  Serial.println("Arduino Nano 33 BLE Sense (Central Device)");
  Serial.println(" ");

  //memset(cumMyo,0,sizeof(cumMyo));
}

void loop() {
  connectToPeripheral();
}

void connectToPeripheral(){
  BLEDevice peripheral;
  
  Serial.println("- Discovering peripheral device...");

  do
  {
    BLE.scanForUuid(deviceServiceUuid);
    peripheral = BLE.available();
  } while (!peripheral);
  
  if (peripheral) {
    Serial.println("* Peripheral device found!");
    Serial.print("* Device MAC address: ");
    Serial.println(peripheral.address());
    Serial.print("* Device name: ");
    Serial.println(peripheral.localName());
    Serial.print("* Advertised service UUID: ");
    Serial.println(peripheral.advertisedServiceUuid());
    Serial.println(" ");
    BLE.stopScan();
    controlPeripheral(peripheral);
  }
}

void controlPeripheral(BLEDevice peripheral) {
  Serial.println("- Connecting to peripheral device...");

  if (peripheral.connect()) {
    Serial.println("* Connected to peripheral device!");
    Serial.println(" ");
  } else {
    Serial.println("* Connection to peripheral device failed!");
    Serial.println(" ");
    return;
  }

  Serial.println("- Discovering peripheral device attributes...");
  if (peripheral.discoverAttributes()) {
    Serial.println("* Peripheral device attributes discovered!");
    Serial.println(" ");
  } else {
    Serial.println("* Peripheral device attributes discovery failed!");
    Serial.println(" ");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic gestureCharacteristic = peripheral.characteristic(deviceServiceCharacteristicUuid);
    
  if (!gestureCharacteristic) {
    Serial.println("* Peripheral device does not have gesture_type characteristic!");
    peripheral.disconnect();
    return;
  } else if (!gestureCharacteristic.canWrite()) {
    Serial.println("* Peripheral does not have a writable gesture_type characteristic!");
    peripheral.disconnect();
    return;
  }
  
  while (peripheral.connected()) {
    //gesture = gestureDetectection();
    motorCommand = simMyo();

    if (oldGestureValue != motorCommand) {  
      oldGestureValue = motorCommand;
      Serial.print("* Writing value to motorCommand characteristic: ");
      Serial.println(motorCommand);
      gestureCharacteristic.writeValue((byte)motorCommand);
      Serial.println("* Writing value to motorCommand characteristic done!");
      Serial.println(" ");
    }
  
  }
  Serial.println("- Peripheral device disconnected!");
}
  
int gestureDetectection() {
  if (APDS.gestureAvailable()) {
    gesture = APDS.readGesture();

    switch (gesture) {
      case GESTURE_UP:
        Serial.println("- UP gesture detected");
        break;
      case GESTURE_DOWN:
        Serial.println("- DOWN gesture detected");
        break;
      case GESTURE_LEFT:
        Serial.println("- LEFT gesture detected");
        break;
      case GESTURE_RIGHT:
        Serial.println("- RIGHT gesture detected");
        break;
      default:
        Serial.println("- No gesture detected");
        break;
      }
    }
    return gesture;
}


int simMyo(){
  //int sensorValue = analogRead(A0);
  int sensorValue = random(1024);
  // delay(100);
  delay(10);

  // queue 
  for (int i = 0; i<19; i++){
    cumMyo[i] = cumMyo[i+1];
  }
  cumMyo[19] = sensorValue;

  int sumMyo = 0;
  for (int i = 0; i<20; i++){
    sumMyo += cumMyo[i];
    //Serial.println(cumMyo[i]);
  }

  
  //if (sensorValue >= 900){
  if (sumMyo >= 12000){
    //cumMyo += 1；
    motorCommand = 1;
  }
  else{
    motorCommand = -1;
  }

  switch (motorCommand) {
      case 1:
        Serial.println("- switch command detected");
        break;

      default:
        Serial.println("- No switch detected");
        break;
  }
    
  return motorCommand;

}




