#include <Arduino.h>
#include <SPIFlash.h>
#include <FXOS8700.h>

#define LED2 9
#define TEST_EIC 18

#define FXOS_MISO 26
#define FXOS_CS 11
#define FXOS_RST 3
#define FXOS_INT1 13
#define FXOS_INT2 14

#define RFM_SS 7
#define FLASH_SS 10

SPIFlash flash( FLASH_SS, 0x1F44 );
uint8_t  buff[256];
uint8_t  typeFlags = 0;
Buffer_t txBuff;

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
    uint32_t mil = millis();
    uint32_t mi = micros();
    uint32_t sec = secondsRTC();

    Serial.print( "Steps: " );
    Serial.println( steps );
    Serial.print( "Micros: " );
    Serial.println( mi );
    Serial.print( "Millis: " );
    Serial.println( mil );
    Serial.print( "Sec: " );
    Serial.println( sec );
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
    EEPROM.write( 0, buff, 64 );
    EEPROM.write( 64, &buff[64], 64 );
    EEPROM.write( 200, buff, 256 );

    memset( buff, 0, 256 );
    EEPROM.read( 0, buff, 128 );
    EEPROM.read( 200, buff, 256 );

    EEPROM.erase( 200, 128 );
    EEPROM.read( 200, buff, 256 );
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
    if( Timer.isActive() ) Timer.pause();
    if( Timer1.isActive() ) Timer1.pause();
    digitalWrite( LED2, LOW );
    enableAPBBClk( PM_APBBMASK_PORT, 0 );
    pauseMicrosForSleep();

    // Sleep the CPU
    sleepCPU( PM_SLEEP_STANDBY_Val );

    // Bring back modules
    Serial.begin( 38400 );
    if( Timer.isActive() ) Timer.resume();
    if( Timer1.isActive() ) Timer1.resume();
}

volatile uint32_t ISRCntr = 0;
void              sleepEICISR()
{
    // digitalWrite( LED2, ( ISRCntr % 2 ) );
    ISRCntr++;
}

void testEIC()
{
    interruptlowPowerMode( true );
    attachInterrupt( TEST_EIC, sleepEICISR, FALLING );
}

bool resetDetected = false;
void resetISR()
{
    resetDetected = true;
}

void testFXOS()
{
    if( fxos.initialize() ) {
        Serial.println( "FXOS Success" );
        Serial.print( "Temp: " );
        Serial.println( fxos.getTemperature() );
    }
}

void testReadVCC()
{
    uint32_t vcc = analogReadVcc();
    Serial.print( "VCC: " );
    Serial.println( vcc );
}

void testReadAnalog()
{
    analogReference( AR_EXTERNAL );
    uint32_t val = analogRead( A5 );
    Serial.print( "Pin A5: " );
    Serial.println( val );
}

volatile uint32_t timerISRCntr = 0;
void              timerCntrISR()
{
    //digitalWrite( LED2, ( timerISRCntr % 2 ) );
    if( ( timerISRCntr % 2 )  )
        PORT->Group[0].OUTCLR.reg = ( 1 << 22 );
    else
        PORT->Group[0].OUTSET.reg = ( 1 << 22 );
    timerISRCntr++;
}

void testTimerCounters( uint8_t size )
{
    if( size == 8 ) {
        Timer.registerISR( timerCntrISR );
        Timer.begin( 20000, -1, tc_mode_8_bit, true );
    }
    if( size == 16 ) {
        Timer.registerISR( timerCntrISR );
        Timer.begin( 20000, -1, tc_mode_16_bit, true );
    }
    if( size == 32 ) {
        //Timer1.registerISR( timerCntrISR );
        Timer1.begin( 20000, true, tc_mode_32_bit, false );
    }
}

void setup()
{
    // Select the clock
    // changeCPUClk( cpu_clk_dfll48 );

    // Initialize Packet Builder
    PacketBuilder::InitPacketBuilder( &typeFlags, &txBuff );

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

    // Initialize SPI Flash
    flash.initialize();

    // Initialize EEPROM
    EEPROM.begin();

    // Setup Timer Counter
    testTimerCounters( 32 );
}

uint32_t sec = 0;
void     loop()
{
    testSleep();
    testSPIFlash();
    testFXOS();
    testEEEPROM();
    testNVMFlash();
    testReadVCC();
    testReadAnalog();
    printRTC();
    delay( 25 );
}