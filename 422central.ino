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

#include "filter_lib.h"

#include <TinyMLShield.h>
#include <SD.h>
#include <SPI.h>

bool commandRecv = false;  // flag used for indicating receipt of commands from serial port
bool liveFlag = false;     // flag as true to live stream raw camera bytes, set as false to take single images on command
bool captureFlag = false;

// Image buffer;
byte image[176 * 144 * 2];  // QCIF: 176x144 x 2 bytes per pixel (RGB565)
int bytesPerFrame;
int r,g,b;
int cumr, cumg, cumb;
float aver = 0, aveg = 0, aveb = 0;
int totalPixel = 176*144;
int BLEcommand;


lowpass_filter lowpassFilter(60);  //set the cuttoff frequency for the filter 

float raw_signal;
float filtered_signal;
//int servo1pos = 0;

enum StatesServo {
  OPEN,
  MIDDLE,
  CLOSED
};

// Set the initial (i.e., starting) state
StatesServo state = StatesServo::OPEN;

void nextStateServo() { 
  if (state == StatesServo::OPEN) state = StatesServo::MIDDLE;
  else if (state == StatesServo::MIDDLE) state = StatesServo::CLOSED;
  else state = StatesServo::OPEN;
}

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a0618";
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a0618";
const char* filterSignalUuid = "19b10002-e8f2-537e-4f6c-d104768a0618";
const char* RUuid = "19b10003-e8f2-537e-4f6c-d104768a0618";
const char* GUuid = "19b10004-e8f2-537e-4f6c-d104768a0618";
const char* BUuid = "19b10005-e8f2-537e-4f6c-d104768a0618";

int gesture = -1;
int oldGestureValue = -2;   
int cumMyo[20] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int motorCommand = -1;
float oldFilterSignal = 0;
float oldR = -1;
float oldG = -1;
float oldB = -1;


void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  if (!APDS.begin()) {
    Serial.println("* Error initializing APDS9960 sensor!");
  } 

  APDS.setGestureSensitivity(80); 

  initializeShield();

  // Initialize the OV7675 camera
  if (!Camera.begin(QCIF, RGB565, 1, OV7675)) {
    Serial.println("Failed to initialize camera");
    while (1)
      ;
  }
  bytesPerFrame = Camera.width() * Camera.height() * Camera.bytesPerPixel();
  
  if (!BLE.begin()) {
    Serial.println("* Starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  
  BLE.setLocalName("Nano 33 BLE (Central)"); 
  BLE.advertise();

  Serial.println("Arduino Nano 33 BLE Sense (Central Device)");
  Serial.println(" ");

  CameraRGB();
  Serial.println(aver);
  Serial.println(aveg);
  Serial.println(aveb);

  //memset(cumMyo,0,sizeof(cumMyo));
}

void loop() {
  
  connectToPeripheral();
}

void CameraRGB(){

  Camera.readFrame(image);
  delay(20);

  cumr = 0;
  cumg = 0;
  cumb = 0;

  aver = 0;
  aveg = 0;
  aveb = 0;

      for (int i = 0; i < Camera.height() - 1; i += 1) {
        for (int j = 0; j < Camera.width()* Camera.bytesPerPixel() - 1; j += 2) {
          int index = i * Camera.width()* Camera.bytesPerPixel() + j;
          
          int p = image[index]*256 + image[index + 1];
          
          rgbdecode(p);

          cumr += r;
          cumg += g;
          cumb += b;
          
        }

      }

      aver = cumr/totalPixel;
      aveg = cumg/totalPixel;
      aveb = cumb/totalPixel;


}

void rgbdecode(int s){
    r = (s & 0xF800) >> 8;
    g = (s & 0x7E0 ) >> 3;
    b = (s & 0x1F) << 3;
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
  BLECharacteristic filteredSignalCharacteristic = peripheral.characteristic(filterSignalUuid);
  BLECharacteristic RCharacteristic = peripheral.characteristic(RUuid);
  BLECharacteristic GCharacteristic = peripheral.characteristic(GUuid);
  BLECharacteristic BCharacteristic = peripheral.characteristic(BUuid);
    
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
    CameraRGB();
    //findState();
    //getMotorCommand();

    if (oldGestureValue != motorCommand) {  
      oldGestureValue = motorCommand;
      Serial.print("* Writing value to motorCommand characteristic: ");
      Serial.println(motorCommand);
      gestureCharacteristic.writeValue((byte)motorCommand);
      Serial.println("* Writing value to motorCommand characteristic done!");
      Serial.println(" ");
    }

    if (oldFilterSignal != filtered_signal) {  
      oldFilterSignal = filtered_signal;
      Serial.print("* Writing value to filtered signal characteristic: ");
      Serial.println(filtered_signal);
      filteredSignalCharacteristic.writeValue((byte)filtered_signal);
      Serial.println("* Writing value to filtered signal characteristic done!");
      Serial.println(" ");
    }

    if (oldR != aver) {  
      oldR = aver;
      RCharacteristic.writeValue((byte)aver);
      Serial.println("* Writing value to Camera RGB characteristic done!");
      Serial.println(" ");
    }

    if (oldG != aveg) {  
      oldG = aveg;
      GCharacteristic.writeValue((byte)aveg);
      Serial.println("* Writing value to Camera RGB characteristic done!");
      Serial.println(" ");
    }

    if (oldB != aveb) {  
      oldB = aveb;
      BCharacteristic.writeValue((byte)aveb);
      Serial.println("* Writing value to Camera RGB characteristic done!");
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
  int simCommand = 0;
  // delay(100);
  delay(20);

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

  filtered_signal = sumMyo/20;
  
  //if (sensorValue >= 900){
  if (sumMyo >= 12000){
    //cumMyo += 1；
    simCommand = 1;
    
  }
  else{
    simCommand = -1;
  }

  switch (simCommand) {
      case 1:
        Serial.println("- switch command detected");
        break;

      default:
        Serial.println("- No switch detected");
        break;
  }
    
  return simCommand;

}

void findState(){
  int raw_signal = analogRead(A6);

  // float t = micros() / 1e6;
  // Serial.println(raw_signal);
  filtered_signal = lowpassFilter.filter(raw_signal);
  // int pixelColor = map(filtered_signal, 0, 1023, 0, 255); // need algorithm to determin hue from this
  Serial.println(filtered_signal);
  
  //filtered_signal > 500
  if (filtered_signal > 400){
    //int startTime = millis();
    //if (filtered_signal > 500 && (millis - startTime  >  2000));
    nextStateServo();

    }
  delay(200);
}

void getMotorCommand(){
  if (state == StatesServo::OPEN) motorCommand = 1;
  else if (state == StatesServo::MIDDLE) motorCommand = 0;
  else motorCommand = -1;
}




