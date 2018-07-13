#include <Arduino.h>
#include <EEPROM.h>
#include <SPIFlash.h>

SPIFlash flash( 10, 0x1F44 );
EEEPROM<NVMFlash> eeeprom;
uint8_t buff[256];

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
    if( millis() > 12000 ) resetCPU();
}

void testNVMFlash()
{
    eraseRow( 0x10000 );
    readFlash( (void *)0x10000, buff, 256 );

    for( uint16_t i = 0; i < 128; i++ ) buff[i] = i & 0xFF;
    writeFlash( (void *)0x10000, buff, 64 );

    memset( buff, 0, 256 );
    readFlash( (void *)0x10000, buff, 128 );

    for( uint16_t i = 0; i < 128; i++ ) buff[i] = i & 0xFF;
    writeFlash( (void *)0x10040, buff, 64 );

    memset( buff, 0, 256 );
    readFlash( (void *)0x10000, buff, 128 );
}

void testSPIFlash()
{
    flash.pageErase256( 0 );
    memset( buff, 0, 256 );
    flash.readBytes( 0, buff, 256 );

    for( uint16_t i = 0; i < 256; i++ ) buff[i] = i & 0xFF;
    flash.writeBytes( 0, buff, 256 );
    
    memset( buff, 0, 256 );
    flash.readBytes( 0, buff, 256 );
}

void testEEEPROM()
{
    for( uint16_t i = 0; i < 256; i++ ) buff[i] = i & 0xFF;
    eeeprom.write( 0, buff, 64 );
    eeeprom.write( 64, &buff[64], 64 );
    eeeprom.write( 200, buff, 256 );

    memset( buff, 0, 256 );
    eeeprom.read( 0, buff, 128 );
    eeeprom.read( 200, buff, 256 );

    eeeprom.erase( 200, 128 );
    eeeprom.read( 200, buff, 256 );
}

void hardMathTest()
{
    uint32_t value = 789456;
    for( uint8_t i = 0; i < 200; i++ ) {
        value /= ( i + 1 );
        value *= 2;
        value++;
    }
}

void testSleep()
{
    enableAPBBClk( PM_APBBMASK_PORT, 0 );

    // Sleep the CPU
    sleepCPU( PM_SLEEP_STANDBY_Val );

    // Print time
    Serial.begin( 115200 );
    Serial.print( "Begin:" );
    Serial.println( millis() );
    testEEEPROM();
    hardMathTest();
    testSPIFlash();
    Serial.print( "End:" );
    Serial.println( millis() );
    delay( 10 );
    Serial.end();
}

void setup()
{
    // Select the clock
    changeCPUClk( cpu_clk_dfll48 );

    // RFM SS
    pinMode( 7, OUTPUT );
    digitalWrite( 7, HIGH );

    // Flash SS
    flash.initialize();
}

void loop()
{
    testSleep();
}