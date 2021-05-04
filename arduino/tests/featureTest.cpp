#include <Arduino.h>
#include <SPI.h>

#if defined( FLUME_GA_WS_BOARD )
#include <FXOS8700_REGISTERS.h>

#define FLASH_SS 10
#define RFM_SS 7
#define FXOS_CS_PIN 11
#define FXOS_SCK_PIN 20
#define FXOS_MOSI_PIN 19
#define FXOS_MISO_PIN 2
#define FXOS_RST_PIN 3
#define FXOS_INT1_PIN 13
#define FXOS_INT2_PIN 14
#define LED1 4
#define LED2 12

volatile uint32_t accumEIC[] = {0, 0, 0};
volatile uint32_t isrCntrEIC = 0;
volatile uint8_t  bytesEIC[6];

#endif /* FLUME_GA_WS_BOARD */

#define PRINT_BUFF_SIZE 2048
#define TC_ISR_SAMPLE_CNT 10

char              _printBuff[PRINT_BUFF_SIZE];
volatile uint32_t _TCISRTimeStamps, _TCISRPrev;
volatile uint8_t  _TCISRIndex = 0;
volatile uint8_t  _NMIISRCntr = 0;

void testSPI();
void testGPIO();
void testDelay();
void testEEPROM();
void testSleep();
void testWDT();
void testClockChange();
void testRapidCPUChange();
void testTCContinuous();
void testAsyncCounter();
void testEIC();
void testAnalog();
void focusDelayTest();
void testProcessingSpeed();
void testWDTClear();

void setup()
{
    Serial.begin( 500000 );
    Serial.println( "Hello" );
}

void loop()
{
    // By doing a command / response switch case with a console we are testing
    // the Serial functionality

    if( Serial.available() ) {
        char c = (char)Serial.read();
        switch( c ) {
            case 'c': testClockChange(); break;
            case 'd': testDelay(); break;
            case 'e': testEEPROM(); break;
            case 'r': resetCPU(); break;
            case 's': testSleep(); break;
            case 't': testTCContinuous(); break;
            case 'w': testWDT(); break;
            case 'g': testGPIO(); break;
            case 'i': testSPI(); break;
            case 'p': testEIC(); break;
            case 'a': testAnalog(); break;
            case 'f': focusDelayTest(); break;
            case 'q': testProcessingSpeed(); break;
            case 'm': testAsyncCounter(); break;
            case 'z': testWDTClear(); break;
            case '1': testRapidCPUChange(); break;
        }
    }

    // delay( 1 );
}

void testAnalog()
{
    Analog  badPin( 4 ), highPinDefault( 13 ), negPinDiff( 17, 13 );
    int32_t val;

    // Try a non-analog pin
    val = badPin.readSingle();
    if( val == -1 ) Serial.println( "Non-analog pin successfully setup" );

    // Should read unsigned max positive value (4095)
    val = highPinDefault.readSingle();
    Serial.println( val );

    // Should read signed max negative value (-2048)
    val = negPinDiff.readSingle();
    Serial.println( (int16_t)val );

    // Should read signed max negative value (-2048)
    val = negPinDiff.readSingle(
        AnalogSettings( ana_ref_internal_1v, ana_resolution_12bit,
                        ana_clk_div_8, ana_accum_64, ana_gain_1x ) );
    Serial.println( (int16_t)val );

    Serial.println( Analog::readVCC() );
    Serial.println( Analog::readTemperature() );

    // Make a triangle wave
    for( uint16_t i = 0; i < 0x3FF; i++ ) highPinDefault.writeSingle( i );
    for( uint16_t i = 0x3FF; i > 0; i-- ) highPinDefault.writeSingle( i );
}

void EICISR()
{
#if defined( FLUME_GA_WS_BOARD )
    // Read the raw data
    Serial.println( ( uint32_t )( stepsRTC() >> 5 ) );
    isrCntrEIC++;

    digitalWrite( FXOS_CS_PIN, LOW );
    SPI1.transfer( M_OUT_X_MSB & 0x7F );
    SPI1.transfer( M_OUT_X_MSB & 0x80 );
    for( uint8_t i = 0; i < 6; i++ ) bytesEIC[i] = SPI1.transfer( 0 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    // Accumulate samples
    for( uint8_t i = 3; i--; )
        accumEIC[i] += ( bytesEIC[i << 1] << 8 ) + bytesEIC[( i << 1 ) + 1];
#endif /* FLUME_GA_WS_BOARD */
}

void testAsyncCounter()
{
    interruptlowPowerMode( true );

    for( uint8_t i = 0; i < 3; i++ ) accumEIC[i] = 0;
    isrCntrEIC = 0;

    // Ensure that the SPI bus is working before we test this (required for the
    // interface)
    testSPI();

    pinMode( FXOS_INT2_PIN, INPUT_PULLUP );
    attachInterrupt( FXOS_INT2_PIN, EICISR, FALLING );

    // Set everything up
    uint8_t regs[] = {
        // Set output data rate to 200 Hz
        CTRL_REG1, ODR_200HZ,
        // Default settings for Accelerometer sleep modes
        CTRL_REG2, 0,
        // Default settings for Accelerometer vector mag (off), active low
        // interrupts
        CTRL_REG3, 0,
        // Enable data ready interrupt
        CTRL_REG4, ( 1 << ctrl_reg4_en_drdy ),
        // Data ready interrupt routed to INT2 (default settings)
        CTRL_REG5, 0,
        // Awake OSR is 4x, enable magnetometer only
        M_CTRL_REG1, ( MOSR_6 | ( 1 << mctrl1_hms0 ) ),
        // Disable max/min detection, degaussing triggered at beginning of each
        // ODR cycle
        M_CTRL_REG2, ( 1 << mctrl2_maxmin_dis ),
        // Use raw values, auto sleep OSR is 2x
        M_CTRL_REG3, ( ( 1 << mctrl3_raw ) | MSOSR_4 ),
        // Disable all threshold IRQs
        M_THS_CFG, 0,
        // Vector magnitude thresholds
        M_VECM_THS_LSB, 0, M_VECM_THS_MSB, 0 >> 8, M_VECM_CNT, 0,
        // Vector magnitude configuration
        M_VECM_CFG, 0, 0};

    // Place in standby to set all the registers
    digitalWrite( FXOS_CS_PIN, LOW );
    uint8_t ctrlReg1 = 0;
    SPI1.transfer( CTRL_REG1 & 0x7F );
    SPI1.transfer( CTRL_REG1 & 0x80 );
    ctrlReg1 = SPI1.transfer( 0 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    ctrlReg1 &= ~( 1 << ctrl_reg1_active );
    digitalWrite( FXOS_CS_PIN, LOW );
    SPI1.transfer( ( CTRL_REG1 & 0x7F ) | 0x80 );
    SPI1.transfer( CTRL_REG1 & 0x80 );
    SPI1.transfer( ctrlReg1 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    // Set all registers
    for( uint8_t i = 0; regs[i] != 0; ) {
        uint8_t reg = regs[i++];
        uint8_t val = regs[i++];
        digitalWrite( FXOS_CS_PIN, LOW );
        SPI1.transfer( ( reg & 0x7F ) | 0x80 );
        SPI1.transfer( reg & 0x80 );
        SPI1.transfer( val );
        digitalWrite( FXOS_CS_PIN, HIGH );
    }

    // Set in active
    digitalWrite( FXOS_CS_PIN, LOW );
    SPI1.transfer( CTRL_REG1 & 0x7F );
    SPI1.transfer( CTRL_REG1 & 0x80 );
    ctrlReg1 = SPI1.transfer( 0 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    ctrlReg1 |= ( 1 << ctrl_reg1_active );
    digitalWrite( FXOS_CS_PIN, LOW );
    SPI1.transfer( ( CTRL_REG1 & 0x7F ) | 0x80 );
    SPI1.transfer( CTRL_REG1 & 0x80 );
    SPI1.transfer( ctrlReg1 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    uint32_t start = millis();
    while( millis() - start < 500000 )
        ;

    // Reset the FXOS and detach the interrupt
    digitalWrite( FXOS_RST_PIN, HIGH );
    digitalWrite( FXOS_RST_PIN, LOW );
    detachInterrupt( FXOS_INT2_PIN );
}

void testEIC()
{
#if defined( FLUME_GA_WS_BOARD )
    uint8_t runThroughs;

    runThroughs = 0;
GO_AGAIN:
    if( runThroughs++ ) interruptlowPowerMode( true );

    for( uint8_t i = 0; i < 3; i++ ) accumEIC[i] = 0;
    isrCntrEIC = 0;

    // Ensure that the SPI bus is working before we test this (required for the
    // interface)
    testSPI();

    pinMode( FXOS_INT2_PIN, INPUT_PULLUP );
    attachInterrupt( FXOS_INT2_PIN, EICISR, FALLING );

    // Set everything up
    uint8_t regs[] = {
        // Set output data rate to 200 Hz
        CTRL_REG1, ODR_200HZ,
        // Default settings for Accelerometer sleep modes
        CTRL_REG2, 0,
        // Default settings for Accelerometer vector mag (off), active low
        // interrupts
        CTRL_REG3, 0,
        // Enable data ready interrupt
        CTRL_REG4, ( 1 << ctrl_reg4_en_drdy ),
        // Data ready interrupt routed to INT2 (default settings)
        CTRL_REG5, 0,
        // Awake OSR is 4x, enable magnetometer only
        M_CTRL_REG1, ( MOSR_6 | ( 1 << mctrl1_hms0 ) ),
        // Disable max/min detection, degaussing triggered at beginning of each
        // ODR cycle
        M_CTRL_REG2, ( 1 << mctrl2_maxmin_dis ),
        // Use raw values, auto sleep OSR is 2x
        M_CTRL_REG3, ( ( 1 << mctrl3_raw ) | MSOSR_4 ),
        // Disable all threshold IRQs
        M_THS_CFG, 0,
        // Vector magnitude thresholds
        M_VECM_THS_LSB, 0, M_VECM_THS_MSB, 0 >> 8, M_VECM_CNT, 0,
        // Vector magnitude configuration
        M_VECM_CFG, 0, 0};

    // Place in standby to set all the registers
    digitalWrite( FXOS_CS_PIN, LOW );
    uint8_t ctrlReg1 = 0;
    SPI1.transfer( CTRL_REG1 & 0x7F );
    SPI1.transfer( CTRL_REG1 & 0x80 );
    ctrlReg1 = SPI1.transfer( 0 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    ctrlReg1 &= ~( 1 << ctrl_reg1_active );
    digitalWrite( FXOS_CS_PIN, LOW );
    SPI1.transfer( ( CTRL_REG1 & 0x7F ) | 0x80 );
    SPI1.transfer( CTRL_REG1 & 0x80 );
    SPI1.transfer( ctrlReg1 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    // Set all registers
    for( uint8_t i = 0; regs[i] != 0; ) {
        uint8_t reg = regs[i++];
        uint8_t val = regs[i++];
        digitalWrite( FXOS_CS_PIN, LOW );
        SPI1.transfer( ( reg & 0x7F ) | 0x80 );
        SPI1.transfer( reg & 0x80 );
        SPI1.transfer( val );
        digitalWrite( FXOS_CS_PIN, HIGH );
    }

    // Set in active
    digitalWrite( FXOS_CS_PIN, LOW );
    SPI1.transfer( CTRL_REG1 & 0x7F );
    SPI1.transfer( CTRL_REG1 & 0x80 );
    ctrlReg1 = SPI1.transfer( 0 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    ctrlReg1 |= ( 1 << ctrl_reg1_active );
    digitalWrite( FXOS_CS_PIN, LOW );
    SPI1.transfer( ( CTRL_REG1 & 0x7F ) | 0x80 );
    SPI1.transfer( CTRL_REG1 & 0x80 );
    SPI1.transfer( ctrlReg1 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    while( isrCntrEIC < 200 )
        ;

    // Reset the FXOS and detach the interrupt
    digitalWrite( FXOS_RST_PIN, HIGH );
    digitalWrite( FXOS_RST_PIN, LOW );
    detachInterrupt( FXOS_INT2_PIN );

    uint8_t i = sprintf(
        _printBuff,
        "Testing EIC\nSaw %lu interrupts, average read values %lu, %lu, %lu",
        isrCntrEIC, accumEIC[0] / isrCntrEIC, accumEIC[1] / isrCntrEIC,
        accumEIC[2] / isrCntrEIC );
    _printBuff[i] = 0;
    Serial.println( _printBuff );

    if( runThroughs == 1 ) goto GO_AGAIN;
#endif /* FLUME_GA_WS_BOARD */
}

void testSPI()
{
#if defined( FLUME_GA_WS_BOARD )
    SPISettings settings0, settings1;
    uint8_t     i;

    // Set up and run SPI channel 0
    pinMode( FLASH_SS, OUTPUT );
    pinMode( RFM_SS, OUTPUT );
    digitalWrite( FLASH_SS, HIGH );
    digitalWrite( RFM_SS, HIGH );

    Serial.println( "Testing SPI channel 0" );
    settings0 = SPISettings( 4000000, MSBFIRST, SPI_MODE0 );

    // Read the flash memory device ID
    SPI.beginTransaction( settings0 );
    digitalWrite( FLASH_SS, LOW );
    SPI.transfer( 0x9F );
    uint32_t rdid;
    SPI.transfer( 0 );
    rdid = SPI.transfer( 0 ) << 24;
    rdid |= SPI.transfer( 0 ) << 16;
    rdid |= SPI.transfer( 0 ) << 8;
    digitalWrite( FLASH_SS, HIGH );
    SPI.endTransaction();

    i = sprintf( _printBuff, "Got %X for flash memory ID", rdid );
    _printBuff[i] = 0;
    Serial.println( _printBuff );

    // Set up and run SPI channel 1, which requires a special reset sequence for
    // the FXOS
    Serial.println( "Testing SPI channel 1" );
    SPI1.end();
    settings1 = SPISettings( 1000000, MSBFIRST, SPI_MODE0 );

    pinMode( FXOS_RST_PIN, OUTPUT );
    digitalWrite( FXOS_RST_PIN, LOW );
    pinMode( FXOS_CS_PIN, OUTPUT );
    digitalWrite( FXOS_CS_PIN, HIGH );

    // Must set the device up for SPI, delay 10 ms
    pinMode( FXOS_MISO_PIN, TRI_STATE );
    delay( 10 );

    // Do the reset
    digitalWrite( FXOS_RST_PIN, HIGH );
    digitalWrite( FXOS_RST_PIN, LOW );

    // Wait 10 ms for the reset to happen;
    delay( 10 );

    // Read the WHO_AM_I register
    uint8_t whoAmI = 0;
    SPI1.beginTransaction( settings1 );
    digitalWrite( FXOS_CS_PIN, LOW );
    SPI1.transfer( 0x0D & 0x7F );
    SPI1.transfer( 0x0D & 0x80 );
    whoAmI = SPI1.transfer( 0 );
    digitalWrite( FXOS_CS_PIN, HIGH );

    i = sprintf( _printBuff, "Got %X for FXOS ID", whoAmI );
    _printBuff[i] = 0;
    Serial.println( _printBuff );
#endif /* FLUME_GA_WS_BOARD */
}

void testGPIO()
{
#if defined( FLUME_GA_WS_BOARD )
    uint8_t led1, led2, fxosCs, flashCs, i;

    Serial.println( "Digital GPIO test, format "
                    "is\n\tMODE\t\t|\tLED1\t|\tLED2\t|\tFX CS\t|\tFL CS\t" );

    // Test output
    pinMode( LED1, OUTPUT );
    pinMode( LED2, OUTPUT );
    pinMode( FXOS_CS_PIN, OUTPUT );
    pinMode( FLASH_SS, OUTPUT );
    led1 = digitalRead( LED1 );
    led2 = digitalRead( LED2 );
    fxosCs = digitalRead( FXOS_CS_PIN );
    flashCs = digitalRead( FLASH_SS );
    i = sprintf( _printBuff, "output low\t\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t\n",
                 led1, led2, fxosCs, flashCs );

    digitalWrite( LED1, HIGH );
    digitalWrite( FXOS_CS_PIN, HIGH );
    led1 = digitalRead( LED1 );
    led2 = digitalRead( LED2 );
    fxosCs = digitalRead( FXOS_CS_PIN );
    flashCs = digitalRead( FLASH_SS );
    i += sprintf( &_printBuff[i],
                  "led1, fx cs hi\t\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t\n", led1,
                  led2, fxosCs, flashCs );

    digitalWrite( LED1, LOW );
    digitalWrite( LED2, HIGH );
    digitalWrite( FXOS_CS_PIN, LOW );
    digitalWrite( FLASH_SS, HIGH );
    led1 = digitalRead( LED1 );
    led2 = digitalRead( LED2 );
    fxosCs = digitalRead( FXOS_CS_PIN );
    flashCs = digitalRead( FLASH_SS );
    i += sprintf( &_printBuff[i],
                  "led2, fl cs hi\t\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t\n", led1,
                  led2, fxosCs, flashCs );

    _printBuff[i] = 0;
    Serial.println( _printBuff );

    // Test input
    pinMode( LED1, INPUT );
    pinMode( LED2, INPUT );
    pinMode( FXOS_CS_PIN, INPUT );
    pinMode( FLASH_SS, INPUT );
    led1 = digitalRead( LED1 );
    led2 = digitalRead( LED2 );
    fxosCs = digitalRead( FXOS_CS_PIN );
    flashCs = digitalRead( FLASH_SS );
    i = sprintf( _printBuff, "input\t\t\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t\n", led1,
                 led2, fxosCs, flashCs );

    // LEDs doesn't matter for input pull up or down because they are tied to
    // VBatt on the other side
    pinMode( FXOS_CS_PIN, INPUT_PULLUP );
    pinMode( FLASH_SS, INPUT_PULLUP );
    led1 = digitalRead( LED1 );
    led2 = digitalRead( LED2 );
    fxosCs = digitalRead( FXOS_CS_PIN );
    flashCs = digitalRead( FLASH_SS );
    i += sprintf( &_printBuff[i],
                  "fl & fx input pullup\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t\n", led1,
                  led2, fxosCs, flashCs );

    pinMode( FXOS_CS_PIN, INPUT_PULLDOWN );
    pinMode( FLASH_SS, INPUT_PULLDOWN );
    led1 = digitalRead( LED1 );
    led2 = digitalRead( LED2 );
    fxosCs = digitalRead( FXOS_CS_PIN );
    flashCs = digitalRead( FLASH_SS );
    i += sprintf( &_printBuff[i],
                  "fl & fx input pulldown\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t\n",
                  led1, led2, fxosCs, flashCs );

    _printBuff[i] = 0;
    Serial.println( _printBuff );
    delay( 50 );
#endif /* FLUME_GA_WS_BOARD */
}

// Test stepsRTC, delayRTCSteps, getCPUSteps, delayCPUSteps
void runStepsTest( void ( *delayFunc )( uint64_t ), uint64_t ( *timeFunc )(),
                   uint64_t results[5][3], uint32_t overFlowValue )
{
    float_t divisor[] = {0, 0.25, 0.5, 1, 2};

    uint32_t startT, endT;

    for( uint8_t i = 0; i < 5; i++ ) {
        // Set the expected delay
        results[i][0] = ( uint32_t )( ( overFlowValue * divisor[i] ) + 1 );

        // Run the delay and time it
        startT = timeFunc();
        delayFunc( results[i][0] );
        endT = timeFunc();

        // Store the results
        results[i][1] = endT - startT; // Actual delay length
        results[i][2] =
            results[i][1] - results[i][0]; // Delta between actual and expected
    }
}

// Test delay, millis, delayMicroseconds, and micros
void runDelayTest( void ( *delayFunc )( uint32_t ), uint32_t ( *timeFunc )(),
                   uint32_t results[5][3], uint32_t overFlowValue )
{
    float_t divisor[] = {0, 0.25, 0.5, 1, 2};

    uint32_t startT, endT;

    for( uint8_t i = 0; i < 5; i++ ) {
        // Set the expected delay
        results[i][0] = ( uint32_t )( ( overFlowValue * divisor[i] ) + 1 );

        // Run the delay and time it
        startT = timeFunc();
        delayFunc( results[i][0] );
        endT = timeFunc();

        // Store the results
        results[i][1] = endT - startT; // Actual delay length
        results[i][2] =
            results[i][1] - results[i][0]; // Delta between actual and expected
    }
}

void testDelay()
{
    uint32_t mResults[5][3], uResults[5][3];
    uint64_t rtcResults[5][3], tixResults[5][3];
    int16_t  mErr[5], uErr[5], rtcErr[5], tixErr[5];

    Serial.end();
    for( uint8_t i = 0; i < 5; i++ ) {
        changeCPUClk( (CPUClkSrc_t)i );

        runDelayTest( delay, millis, mResults, 1000 );
        runDelayTest( delayMicroseconds, micros, uResults, 1000000 );
        runStepsTest( delayRTCSteps, stepsRTC, rtcResults, RTC_STEPS_PER_SEC );
        runStepsTest( delayCPUTicks, getCPUTicks, tixResults,
                      SYS_TICK_UNDERFLOW );

        mErr[i] = uErr[i] = 0;
        rtcErr[i] = tixErr[i] = 0;
        for( uint8_t j = 0; j < 5; j++ ) {
            mErr[i] += mResults[j][2];
            uErr[i] += uResults[j][2];
            rtcErr[i] += rtcResults[j][2];
            tixErr[i] += tixResults[j][2];
        }

        mErr[i] /= 5;
        uErr[i] /= 5;
        rtcErr[i] /= 5;
        tixErr[i] /= 5;
    }

    Serial.begin( 500000 );
    uint8_t j = sprintf( _printBuff, "%13s\t|%13s\t|%13s\t|%13s\t|%13s\t|\n",
                         "CPU Freq MHz", "Millis Err", "Micros Err", "RTC Err",
                         "CPU Tix Err" );
    _printBuff[j] = 0;
    Serial.println( "Delay test results" );
    Serial.print( _printBuff );

    for( uint8_t i = 0; i < 5; i++ ) {
        uint8_t freq = 48;
        if( i < 4 ) freq = 8 >> i;
        j = sprintf( _printBuff, "%13d\t|%13d\t|%13d\t|%13d\t|%13d\t|\n", freq,
                     mErr[i], uErr[i], rtcErr[i], tixErr[i] );
        _printBuff[j] = 0;
        Serial.print( _printBuff );
    }
}

// Test the Emulated EEPROM functionality, this also directly tests NVM write
// and read abilities as the Emulated EEPROM is built on the NVM abstraction
// layer.
void testEEPROM()
{
    EEPROM.begin();

    if( EEPROM.getSize() == 0 )
        Serial.println( "EEPROM space not allocated, please set in fuses" );
    else {

        // Test bulk pattern
        char     memStr[] = "Guacamole";
        char     memCmpStr[sizeof( memStr )];
        uint16_t i, j;

        // Write the pattern to emulated EEPROM
        for( i = 0; i < EEPROM.getSize(); ) {
            EEPROM.write( i, memStr, sizeof( memStr ) );
            i += sizeof( memStr );
        }

        j = sprintf( _printBuff, "Wrote %u bytes to EEPROM", i );
        _printBuff[j] = 0;
        Serial.println( _printBuff );

        // Read the pattern back
        for( i = 0; i < EEPROM.getSize(); ) {
            EEPROM.read( i, memCmpStr, sizeof( memStr ) );
            if( memcmp( memStr, memCmpStr, sizeof( memStr ) ) != 0 ) {
                j = sprintf( _printBuff,
                             "mismatch at %u, expected: %s, got: %s", i, memStr,
                             memCmpStr );
                _printBuff[j] = 0;
                Serial.println( _printBuff );
                break;
            }

            i += sizeof( memStr );
        }

        j = sprintf( _printBuff, "Read %u bytes to EEPROM", i );
        _printBuff[j] = 0;
        Serial.println( _printBuff );
    }

    EEPROM.end();
}

// Test sleep functionality, go to sleep until the RTC wakes us up ( 1 second
// intervals )
void testSleep()
{
    uint32_t     wakeTimes[4], slpTimes[4];
    SleepLevel_t slp[] = {_cpu, _cpu_ahb, _cpu_ahb_apb, _deep_sleep};

    // In the higher sleep levels UART transaction and SysTick underflows can
    // wake the processor up
    Serial.end();
    disableSysTick();
    for( uint8_t i = 0; i < 4; i++ ) {
        slpTimes[i] = millis();
        sleepCPU( slp[i] );
        wakeTimes[i] = millis();
        delay( 1 );
    }

    Serial.begin( 500000 );
    Serial.println( "CPU slept at different levels\nSleep level | Sleep Time | "
                    "Wake Time\n" );
    for( uint8_t i = 0; i < 4; i++ ) {
        uint8_t j = sprintf( _printBuff, "%d\t|\t%lu\t|\t%lu\t\n", slp[i],
                             slpTimes[i], wakeTimes[i] );
        _printBuff[j] = 0;
        Serial.println( _printBuff );
    }
}

// Tests the watch dog timer and ensures a reset occurs when it should
void testWDT()
{
    uint32_t timeOut;
    uint8_t  i;

    while( millis() % 1000 != 0 )
        ;
    timeOut = millis() + 8000;

    initWDT( wdt_8_s );
    while( true ) {
        if( millis() % 1000 == 0 ) {
            i = sprintf( _printBuff, "Reset in %lu ms",
                         ( timeOut - millis() ) );
            _printBuff[i] = 0;
            Serial.println( _printBuff );
            delay( 1 );
        }
    }
}

// Tests the ability to change the main clock source and frequency
void testClockChange()
{
    uint64_t tixResults[5][3];
    int16_t  tixErr[5];
    uint32_t clkChangeTime[5];

    Serial.end();
    for( uint8_t i = 0; i < 5; i++ ) {
        clkChangeTime[i] = millis();
        changeCPUClk( (CPUClkSrc_t)i );
        clkChangeTime[i] = millis() - clkChangeTime[i];
        runStepsTest( delayCPUTicks, getCPUTicks, tixResults,
                      SYS_TICK_UNDERFLOW );

        tixErr[i] = 0;
        for( uint8_t j = 0; j < 5; j++ ) tixErr[i] += tixResults[j][2];

        tixErr[i] /= 5;
    }

    Serial.begin( 500000 );
    Serial.println(
        "Changing CPU Frequency, all delay errors should be very "
        "close in CPU ticks\nCPU Freq | Avg Delay Err | Delta time change" );
    for( uint8_t i = 0; i < 5; i++ ) {
        uint8_t freq = 48;
        if( i < 4 ) freq = 8 >> i;
        uint8_t j = sprintf( _printBuff, "%d MHz\t | \t%d | \t%d\n", freq,
                             tixErr[i], clkChangeTime[i] );
        _printBuff[j] = 0;
        Serial.println( _printBuff );
    }
}

void testRapidCPUChange()
{
    // Test switch from 8 MHz to 48 MHz and back
    Serial.end();

    // Start at 8 MHz
    changeCPUClk( cpu_clk_oscm8 );

    // Bump to 48 MHz
    uint32_t st = millis();
    digitalWrite( LED1, LOW );
    changeCPUClk( cpu_clk_dfll48 );
    digitalWrite( LED1, HIGH );
    uint32_t mid = millis() - st;

    // Take down to 8 MHz
    st = millis();
    digitalWrite( LED1, LOW );
    changeCPUClk( cpu_clk_oscm8 );
    digitalWrite( LED1, HIGH );
    uint32_t end = millis() - st;

    Serial.begin( 500000 );
    uint8_t j = sprintf(
        _printBuff, "Fast change CPU clk: 8 -> 48 MHz: %d, 48 -> 8 MHz: %d\n",
        mid, end );
    _printBuff[j] = 0;
    Serial.println( _printBuff );
}

// Interrupt service routine for the timer counter continuous mode testing
void timerISR()
{
    uint32_t m = micros();
    _TCISRTimeStamps += ( m - _TCISRPrev );
    _TCISRPrev = m;
    if( _TCISRIndex++ >= TC_ISR_SAMPLE_CNT ) Timer.pause();
}

// Test the timer counter in continuous mode, utilizes 8, 16, and 32 bit counter
// mode to produce a 2 kHz timer. Also tests interrupt service routines
void testTCContinuous()
{
    int8_t i = (int8_t)tc_mode_32_bit;

    // Test ISR mode 8 through 32 bit mode
    do {
        // Kill all timer modules
        Timer.end();
        Timer.deregisterISR();

        // Reset the counters
        _TCISRIndex = 0;
        _TCISRTimeStamps = 0;
        _TCISRPrev = micros();

        // Set up for 2KHz with ISR, output the signal on pin 5
        Timer.registerISR( timerISR );
        Timer.begin( 2000, false, (TCMode_t)i, true );

        // Wait for the ISRs to accumulate enough values
        while( !Timer.isPaused() )
            ;

        // Should be 500 us
        _TCISRTimeStamps /= TC_ISR_SAMPLE_CNT;
        uint8_t j = sprintf( _printBuff, "Got %lu us between ISRs for mode %d",
                             _TCISRTimeStamps, i );
        _printBuff[j] = 0;
        Serial.println( _printBuff );
        i--;
    } while( i >= 0 );
}

volatile uint32_t dLen;
void              WDTTimeOutTrap( uint32_t lr )
{
    disableWDT();
    pinMode( LED1, OUTPUT );
    pinMode( LED2, OUTPUT );
    digitalWrite( LED1, LOW );
    digitalWrite( LED2, LOW );
    while( 1 )
        ;
}

void focusDelayTest()
{
#if defined( _DEBUG_RTC_ )
    extern RTCDebugStuff_t rtcDBG;

    delayRTCSteps( 38 );

    // Print out of start and steps
    uint8_t j =
        sprintf( _printBuff, "\n\tstart: %lu\n\tduration: ", rtcDBG.start );
    _printBuff[j] = 0;
    Serial.print( _printBuff );
    Serial.println( (uint32_t)rtcDBG.delta );

    // Number of comparisons and overflows
    j = sprintf( _printBuff, "\n\toverflows: %d\n\tcomparisons: %d",
                 rtcDBG.ovfNdx, rtcDBG.compNdx );
    _printBuff[j] = 0;
    Serial.println( _printBuff );

    // Loop through overflows
    for( uint8_t i = 0; i < rtcDBG.ovfNdx; i++ ) {
        j = sprintf( _printBuff,
                     "\n\t\tovf start: %d\n\t\tovf end: %d\n\t\tovf delta: %d",
                     rtcDBG.ovfStepStart[i] & RTC_STEPS_OVERFLOW,
                     rtcDBG.ovfStepEnd[i] & RTC_STEPS_OVERFLOW,
                     rtcDBG.ovfStepEnd[i] - rtcDBG.ovfStepStart[i] );
        _printBuff[j] = 0;
        Serial.println( _printBuff );
    }

    // Loop through comparisons
    for( uint8_t i = 0; i < rtcDBG.compNdx; i++ ) {
        j = sprintf(
            _printBuff,
            "\n\t\tcomp start: %d\n\t\tcomp end: %d\n\t\tcomp delta: %d",
            rtcDBG.compStepStart[i] & RTC_STEPS_OVERFLOW,
            rtcDBG.compStepEnd[i] & RTC_STEPS_OVERFLOW,
            rtcDBG.compStepEnd[i] - rtcDBG.compStepStart[i] );
        _printBuff[j] = 0;
        Serial.println( _printBuff );
    }

    // Loop through ISR
    for( uint8_t i = 0; i < rtcDBG.isrNdx; i++ ) {
        j = sprintf( _printBuff, "\n\t\tisr flags: %04x\n\t\tisr en: %04x",
                     rtcDBG.isrFlags[i], rtcDBG.enFlags[i] );
        _printBuff[j] = 0;
        Serial.println( _printBuff );
    }

    // Print the end time
    j = sprintf( _printBuff, "\n\tend: %lu", rtcDBG.end );
    Serial.println( _printBuff );
    Serial.println();
#endif /* _DEBUG_RTC_ */
}

void testProcessingSpeed()
{
    int32_t  filterResponse[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    int32_t  sampleSignal[512], results[512];
    uint32_t processingDuration[5];

    Serial.end();
    for( uint8_t i = 0; i < 5; i++ ) {
        changeCPUClk( (CPUClkSrc_t)i );
        uint32_t start = micros();
        for( int16_t j = 0; j < 12; j++ ) {
            for( int16_t k = 0; k < 512; k++ ) {
                results[k] += sampleSignal[k] * filterResponse[j];
            }
        }

        processingDuration[i] = micros() - start;
    }

    Serial.begin( 500000 );
    Serial.println(
        "Changing CPU Frequency, calculation times \nCPU Freq | Calc Time us" );
    for( uint8_t i = 0; i < 5; i++ ) {
        uint8_t freq = 48;
        if( i < 4 ) freq = 8 >> i;
        uint8_t j = sprintf( _printBuff, "%d MHz\t | \t%d\t\n", freq,
                             processingDuration[i] );
        _printBuff[j] = 0;
        Serial.println( _printBuff );
    }

    Serial.println( results[0] );
}

void testWDTClear()
{
    initWDT( wdt_8_s );
    for( uint8_t i = 0; i < 32; i++ ) {
        uint32_t delta = micros();
        clearWDT();
        delta = micros() - delta;

        uint8_t j = sprintf( _printBuff, "Clear WDT took: %d", delta );
        _printBuff[j] = 0;
        Serial.println( _printBuff );
    }
    endWDT();
}
