/*
  Written by Warren Woolsey

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sam.h"
#include "clocks.h"
#include "RTC.h"
#include "micros.h"
#include "Arduino.h"

// TODO: Hot fix on micros, remove this later
#include "delay.h"

#define MICROS_TIMER_FREQ 1UL
#define RTC_US_PER_COUNT 30UL
#define MICROS_IN_SEC 1000000UL
#define MICROS_WAIT_SYNC                          \
    {                                             \
        while( TC0->COUNT32.STATUS.bit.SYNCBUSY ) \
            ;                                     \
    }
#define MICROS_SET_READ                     \
    {                                       \
        TC0->COUNT32.READREQ.bit.RCONT = 1; \
        TC0->COUNT32.READREQ.bit.RREQ = 1;  \
        MICROS_WAIT_SYNC;                   \
    }

uint32_t         _cyclesPerUs = 0;
uint8_t          _isPaused = 0;
uint8_t          _microsIsInit = 0;
volatile int64_t _microsSec = 0;

/* Micros implementation that runs independent from the RTC. In order
 * to save maximum power, the Systick module has been disabled so that
 * the processor doesn't wake every ms. millis and delay implementations
 * are based off the RTC, however the RTC doesn't run at a high enough
 * frequency to provide the required resolution of micro seconds. This
 * implementation utilizes a 32 bit Timer Counter which is synchronized
 * to the RTC every time the RTC overflows, thus guaranteeing that drift between
 * this Timer Counter and the RTC is minimized.
 *
 * The user can pause the micros timer before sleeping the CPU in it's deepest
 * sleep setting, when the CPU awakes, the caller will be responsible for
 * restarting, and synchronizing the Timer Counter with the RTC. This can
 * be done by calling pauseMicrosForSleep() and syncMicrosToRTC() respectively.
 */
int8_t initMicros()
{
    uint64_t ccValue;
    uint32_t preScaleBits = 0;
    uint32_t ctrlA = 0;
    uint32_t _maxFreq = SystemCoreClock / 2;

    if( _maxFreq < MICROS_IN_SEC ) return -1;

    enableAPBCClk( PM_APBCMASK_TC0, 1 );
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK0_Val, GCLK_CLKCTRL_ID_TC0_TC1_Val );
    NVIC_EnableIRQ( TC0_IRQn );

    // Reset
    TC0->COUNT32.CTRLA.reg = TC_CTRLA_SWRST;
    while( TC0->COUNT32.CTRLA.bit.SWRST )
        ;

    // Toggle mode
    ctrlA |= ( TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_MODE_COUNT32 );

    // Determine capture compare value
    ccValue = _maxFreq / MICROS_TIMER_FREQ - 1;
    preScaleBits = TC_CTRLA_PRESCALER_DIV1;

    uint8_t i = 0;

    while( i <= 9 ) {
        ccValue = ( ( _maxFreq / MICROS_TIMER_FREQ / ( 2 << i ) ) * 2 ) - 1;
        if( ccValue < 0xFFFFFFFF ) break;
        i++;
        if( i == 4 || i == 6 ||
            i == 8 ) // DIV32 DIV128 and DIV512 are not available
            i++;
    }

    switch( i ) {
        case 0: preScaleBits = TC_CTRLA_PRESCALER_DIV2; break;
        case 1: preScaleBits = TC_CTRLA_PRESCALER_DIV4; break;
        case 2: preScaleBits = TC_CTRLA_PRESCALER_DIV8; break;
        case 3: preScaleBits = TC_CTRLA_PRESCALER_DIV16; break;
        case 5: preScaleBits = TC_CTRLA_PRESCALER_DIV64; break;
        case 7: preScaleBits = TC_CTRLA_PRESCALER_DIV256; break;
        case 9: preScaleBits = TC_CTRLA_PRESCALER_DIV1024; break;
        default: break;
    }

    // Set CTRLA
    ctrlA |= preScaleBits;
    TC0->COUNT32.CTRLA.reg |= ctrlA;
    MICROS_WAIT_SYNC;

    // Set CC
    _cyclesPerUs = ( ccValue + 1 ) / ( MICROS_IN_SEC / 2 );
    TC0->COUNT32.CC[0].reg = (uint32_t)ccValue;
    MICROS_WAIT_SYNC;

    // Allow continuous reads
    MICROS_SET_READ;
    TC0->COUNT32.INTENSET.bit.MC0 = 1;

    // Enable the module and interrupts
    TC0->COUNT32.CTRLA.bit.ENABLE = 1;
    MICROS_WAIT_SYNC;

    _isPaused = 0;
    _microsIsInit = 1;

    return 0;
}

void endMicros()
{
    NVIC_DisableIRQ( TC0_IRQn );
    // Reset
    TC0->COUNT32.CTRLA.reg = TC_CTRLA_SWRST;
    while( TC0->COUNT32.CTRLA.bit.SWRST )
        ;

    disableGenericClk( GCLK_CLKCTRL_ID_TC0_TC1_Val );
    enableAPBCClk( PM_APBCMASK_TC0, 0 );
}

void pauseMicrosForSleep()
{
    if( !_isPaused ) {
        NVIC_DisableIRQ( TC0_IRQn );

        TC0->COUNT32.CTRLA.bit.ENABLE = 0;
        MICROS_WAIT_SYNC;
        _isPaused = 1;
    }
}

void resumeMicrosFromSleep()
{
    if( _isPaused ) {
        NVIC_EnableIRQ( TC0_IRQn );

        TC0->COUNT32.CTRLA.bit.ENABLE = 1;
        MICROS_WAIT_SYNC;
        _isPaused = 0;
    }
}

volatile uint32_t microsForceRead;

void syncMicrosToRTC( uint8_t overFlow )
{
    if( overFlow || _isPaused ) {
        resumeMicrosFromSleep();

        uint32_t counts = RTC->MODE1.COUNT.reg;
        counts *= ( _cyclesPerUs * RTC_US_PER_COUNT );

        MICROS_WAIT_SYNC;
        TC0->COUNT32.COUNT.reg = counts;

        MICROS_SET_READ;
        microsForceRead = TC0->COUNT32.COUNT.reg;
    }
}

int64_t micros()
{
    int64_t mics;
    if( _microsIsInit )
        mics = ( ( _microsSec * 1000000UL ) +
                 ( TC0->COUNT32.COUNT.reg ) / _cyclesPerUs );
    else
        mics = RTC_ROUGH_STEPS_TO_MICROS( stepsRTC() );
    return mics;
}

void delayMicroseconds( uint32_t us )
{
    if( _microsIsInit ) {
        int64_t start = micros();
        while( ( micros() - start ) < us )
            ;
    }
    else
        delayRTCSteps( RTC_ROUGH_MICROS_TO_STEPS( us ) );
}

void micros_IRQHandler()
{
    // test = TC0->COUNT32.COUNT.reg;
    // MICROS_WAIT_SYNC;
    // TC0->COUNT32.COUNT.reg = 0;

    _microsSec++;
    TC0->COUNT32.INTFLAG.bit.MC0 = 1;

    // MICROS_SET_READ;
    // microsForceRead = TC0->COUNT32.COUNT.reg;
}