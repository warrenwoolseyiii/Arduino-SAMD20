/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

/*End of auto generated code by Atmel studio */

#include <Wire.h>
#include <SPI.h>
//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  Wire.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  uint32_t time = millis();
  Serial.print(time);
  delay(500);
}
