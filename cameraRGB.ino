/*
  Active Learning Labs
  Harvard University 
  tinyMLx - OV7675 Camera Test

*/

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
File myFile;

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  initializeShield();

  // Initialize the OV7675 camera
  if (!Camera.begin(QCIF, RGB565, 1, OV7675)) {
    Serial.println("Failed to initialize camera");
    while (1)
      ;
  }
  bytesPerFrame = Camera.width() * Camera.height() * Camera.bytesPerPixel();
  Serial.println(Camera.width());
  Serial.println(Camera.height());
  Serial.println(Camera.bytesPerPixel());
  Serial.println(Camera.bitsPerPixel());

  Serial.println("Welcome to the OV7675 test\n");
  Serial.println("Available commands:\n");
  Serial.println("single - take a single image and print out the hexadecimal for each pixel (default)");
  Serial.println("live - the raw bytes of images will be streamed live over the serial port");
  Serial.println("capture - when in single mode, initiates an image capture");

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  
}

void loop() {
  int i = 0;
  String command;

  bool clicked = readShieldButton();
  if (clicked) {
    if (!liveFlag) {
      if (!captureFlag) {
        captureFlag = true;
      }
    }
  }

  // Read incoming commands from serial monitor
  while (Serial.available()) {
    char c = Serial.read();
    if ((c != '\n') && (c != '\r')) {
      command.concat(c);
    } else if (c == '\r') {
      commandRecv = true;
      command.toLowerCase();
    }
  }

  // Command interpretation
  if (commandRecv) {
    commandRecv = false;
    if (command == "live") {
      Serial.println("\nRaw image data will begin streaming in 5 seconds...");
      liveFlag = true;
      delay(5000);
    } else if (command == "single") {
      Serial.println("\nCamera in single mode, type \"capture\" to initiate an image capture");
      liveFlag = false;
      delay(200);
    } else if (command == "capture") {
      if (!liveFlag) {
        if (!captureFlag) {
          captureFlag = true;
        }
        delay(200);
      } else {
        Serial.println("\nCamera is not in single mode, type \"single\" first");
        delay(1000);
      }
    }
  }

  if (liveFlag) {
    Camera.readFrame(image);
    Serial.write(image, bytesPerFrame);
  } else {
    if (captureFlag) {
      captureFlag = false;
      Camera.readFrame(image);
      Serial.println("\nImage data will be printed out in 3 seconds...");
      myFile.println("testing 1, 2, 3.");
      myFile.close();
      delay(3000);
      /*for (int i = 0; i < bytesPerFrame - 1; i += 2) {
        Serial.print("0x");
        Serial.print(image[i + 1], HEX);
        Serial.print(image[i], HEX);
        if (i != bytesPerFrame - 2) {
          Serial.print(", ");
        }
      }*/


      for (int i = 0; i < Camera.height() - 1; i += 1) {
        for (int j = 0; j < Camera.width()* Camera.bytesPerPixel() - 1; j += 2) {
          int index = i * Camera.width()* Camera.bytesPerPixel() + j;
          //Serial.print("0x");
          Serial.print(image[index + 1]);
          Serial.print(" ");
          Serial.print(image[index]);   
          Serial.print(" ");       
          //Serial.print(", ");
          
          int p = image[index + 1]*256 + image[index];
          
          //rgb5652matrix(p);
          //Serial.print(r);   
          //Serial.print(" ");
          //Serial.print(g);
          //Serial.print(" ");
          //Serial.print(b);       
          //Serial.print(" ");
          
        }
        Serial.println();
      }
      Serial.println();
    }
  }
}

void rgb5652matrix(int p){
  r = 0;
  g = 0;
  b = 0;
  for (int i = 0; i < 5; i += 1) {
    r += bitRead(p,i+11)*2^i;
  }
  for (int i = 0; i < 6; i += 1) {
    g += bitRead(p,i+5)*2^i;
  }
  for (int i = 0; i < 5; i += 1) {
    b += bitRead(p,i)*2^i;
  }

}