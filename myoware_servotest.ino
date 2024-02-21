
#include <Servo.h>

Servo servo1;
int sensorvalue = 0;
int mapped_value;
int servo1pos = 0;

void setup ()
{
  servo1.attach(9);
  Serial.begin(115200);
  while (!Serial); // optionally wait for serial terminal to open
 
}

void loop() 
{  
  sensorvalue = analogRead(A0); // read the input on analog pin A0
  Serial.println(sensorvalue); // print out the value you read
  mapped_value = map(sensorvalue, 0, 1023, 0, 180); //map values to servo angles 
  servo1.write(mapped_value); //write servo to the mapped value 
  delay(200); // to avoid overloading 
}
