#include <Arduino.h>
#include <EEPROM.h>
#include <SPIFlash.h>

#define LED2        9
#define TEST_EIC    18

#define FXOS_MISO   26
#define FXOS_CS     11
#define FXOS_RST    3
#define FXOS_INT1   13
#define FXOS_INT2   14

#define RFM_SS      7
#define FLASH_SS    10

SPIFlash flash( FLASH_SS, 0x1F44 );
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
    // Take down all modules
    SPI.end();
    SPI1.end();
    Serial.end();
    enableAPBBClk( PM_APBBMASK_PORT, 0 );
    pauseMicrosForSleep();

    // Sleep the CPU
    sleepCPU( PM_SLEEP_STANDBY_Val );

    // Bring back modules 
    Serial.begin( 38400 );
}

volatile uint32_t ISRCntr = 0;
void sleepEICISR()
{
    //digitalWrite( LED2, HIGH );
    //delay( 1 );
    //digitalWrite( LED2, LOW );    
    ISRCntr++;
}

bool resetDetected = false;
void resetISR()
{
    resetDetected = true;
}

void testFXOSReset()
{
    resetDetected = false;
    interruptlowPowerMode( false );
    attachInterrupt( FXOS_INT1, resetISR, CHANGE );

    // Tri-state MISO
    pinMode( FXOS_MISO, TRI_STATE );
    delay( 10 );

    // Reset the FXOS
    digitalWrite( FXOS_RST, HIGH );
    digitalWrite( FXOS_RST, LOW );
    delay( 1 );

    while( !resetDetected );

    detachInterrupt( FXOS_INT1 );
    disableExternalInterrupts();
}

void FXOSSPI()
{
    testFXOSReset();

    // Set up the bus
    SPISettings _settings = SPISettings( 1000000, MSBFIRST, SPI_MODE0 );
    SPI1.beginTransaction( _settings );

    // Read WHO_AM_I
    uint8_t tx[3] = { ( 0x0D & 0x7F ), ( 0x0D & 0x80 ) , 0xFF };
    digitalWrite( FXOS_CS, LOW );

    for( uint8_t i = 0; i < 3; i++)
        buff[i] = SPI1.transfer( tx[i] );

    digitalWrite( FXOS_CS, HIGH );
    SPI1.endTransaction();
    SPI1.end();
}

void testADC()
{
    uint32_t vcc = analogReadVcc();
    Serial.print( "VCC: " );
    Serial.println( vcc );
}

void setup()
{
    // Select the clock
    //changeCPUClk( cpu_clk_dfll48 );

    // FXOS CS
    pinMode( FXOS_CS, OUTPUT );
    digitalWrite( FXOS_CS, HIGH );

    // RFM SS
    pinMode( RFM_SS, OUTPUT );
    digitalWrite( RFM_SS, HIGH );

    // FLASH SS
    pinMode( FLASH_SS, OUTPUT );
    digitalWrite( FLASH_SS, HIGH );

    // FXOS_RST
    pinMode( FXOS_RST, OUTPUT );
    digitalWrite( FXOS_RST, LOW );

    // LED2
    pinMode( LED2, OUTPUT );
    digitalWrite( LED2, LOW );

    // Attach the low power interrupt controller for testing
    interruptlowPowerMode( true );
    attachInterrupt( TEST_EIC, sleepEICISR, FALLING );

    // Init SPI Flash
    flash.initialize();
}

uint32_t sec = 0;
void loop()
{
    testSleep();
    if( secondsRTC() > sec ) {
        sec = secondsRTC();
        Serial.print( millis() );
        Serial.print( ": " );
        Serial.println( ISRCntr );
        delay( 10 );
    }

    testSPIFlash();
}