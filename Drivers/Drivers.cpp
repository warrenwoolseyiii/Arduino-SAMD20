#include <Arduino.h>

void initClkOut()
{
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

void printRTC()
{
  uint32_t steps = stepsRTC();
  Serial.print( "Steps: " );
  Serial.println( steps );
  Serial.print( "Micros: " );
  Serial.println( RTC_ROUGH_STEPS_TO_MICROS( steps ) );
  Serial.print( "Millis: " );
  Serial.println( RTC_ROUGH_STEPS_TO_MILLIS( steps ) );
  Serial.print( "Sec: " );
  Serial.println( secondsRTC() );
}

void setup()
{
  initClkOut();
  Serial.begin(9600);
}

void loop()
{
  printRTC();
  delay( 500 );
  delay( 0 );
}