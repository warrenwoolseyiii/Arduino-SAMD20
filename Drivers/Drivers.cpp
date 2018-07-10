#include <Arduino.h>
#include <EEPROM.h>

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
  Serial.println( micros() );
  Serial.print( "Millis: " );
  Serial.println( millis() );
  Serial.print( "Sec: " );
  Serial.println( secondsRTC() );
}

void testWDTReset()
{
  if( millis() > 12000 )
    resetCPU();
}

void testNVMFlash()
{
  uint8_t buff[256];

  eraseRow( 0x10000 );
  readFlash( (void *)0x10000, buff, 256 );

  for( uint16_t i = 0; i < 128; i++ )
    buff[i] = i & 0xFF;
  writeFlash( (void *)0x10000, buff, 64 );
  
  memset( buff, 0, 256 );
  readFlash( (void *)0x10000, buff, 128 );

  for( uint16_t i = 0; i < 128; i++ )
    buff[i] = i & 0xFF;
  writeFlash( (void *)0x10040, buff, 64 );

  memset( buff, 0, 256 );
  readFlash( (void *)0x10000, buff, 128 );
}

EEEPROM<NVMFlash> eeeprom;
void testEEEPROM()
{
  uint8_t buff[256];
  for( uint16_t i = 0; i < 256; i++ )
    buff[i] = i & 0xFF;
  eeeprom.write( 0, buff, 64 );
  eeeprom.write( 64, &buff[64], 64 );
  eeeprom.write( 200, buff, 256 );
  
  memset( buff, 0, 256 );
  eeeprom.read( 0, buff, 128 );
  eeeprom.read( 200, buff, 256 );

  eeeprom.erase( 200, 128 );
  eeeprom.read ( 200, buff, 256 );
}

void testSleep()
{
  sleepCPU( 0xFF );
}

void setup()
{
  //initWDT( WDT_CONFIG_PER_4K_Val );
}

void loop()
{
  //clearWDT();
  testEEEPROM();
  testSleep();
}