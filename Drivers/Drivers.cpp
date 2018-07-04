#include <Arduino.h>

void setup()
{
  Serial.begin(9600);

  // For now output GCLK0 onto pin 3
  pinMode( 3, OUTPUT );
  PORT->Group[0].PINCFG[14].bit.PMUXEN = 1;
  PORT->Group[0].PMUX[7].bit.PMUXE = 0x7;

  // For now output GCLK1 onto pin 4
  pinMode( 4, OUTPUT );
  PORT->Group[0].PINCFG[15].bit.PMUXEN = 1;
  PORT->Group[0].PMUX[7].bit.PMUXO = 0x7;

  // For now output GCLK2 onto pin 5
  pinMode( 5, OUTPUT );
  PORT->Group[0].PINCFG[16].bit.PMUXEN = 1;
  PORT->Group[0].PMUX[8].bit.PMUXE = 0x7;
}

void loop()
{
  Serial.write( "Hello\n" );
  delay(1000);
}