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

#define MICROS_TIMER_FREQ 1UL
#define RTC_US_PER_COUNT 30UL

uint32_t _cyclesPerUs = 0;
uint8_t  _isPaused = 0;
uint32_t _micros = 0;

int8_t initMicros()
{
    uint64_t ccValue;
    uint32_t preScaleBits = 0;
    uint32_t ctrlA = 0;
    uint32_t _maxFreq = SystemCoreClock / 2;

    if( _maxFreq < 1000000UL ) return -1;

    enableAPBCClk( PM_APBCMASK_TC0, 1 );
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK0_Val, GCLK_CLKCTRL_ID_TC0_TC1_Val );

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
        ccValue = _maxFreq / MICROS_TIMER_FREQ / ( 2 << i ) - 1;
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
    while( TC0->COUNT32.STATUS.bit.SYNCBUSY )
        ;

    // Set CC
    _cyclesPerUs = ( ccValue + 1 ) / 500000UL;
    TC0->COUNT32.CC[0].reg = (uint32_t)ccValue;
    while( TC0->COUNT32.STATUS.bit.SYNCBUSY )
        ;

    // Allow continuous reads
    TC0->COUNT32.READREQ.bit.RCONT = 1;

    // Enable the module and interrupts
    TC0->COUNT32.CTRLA.bit.ENABLE = 1;
    while( TC0->COUNT32.STATUS.bit.SYNCBUSY )
        ;

    _isPaused = 0;

    return 0;
}

void endMicros()
{
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
        TC0->COUNT32.CTRLA.bit.ENABLE = 0;
        while( TC0->COUNT32.STATUS.bit.SYNCBUSY )
            ;
        _isPaused = 1;
    }
}

void resumeMicrosFromSleep()
{
    if( _isPaused ) {
        TC0->COUNT32.CTRLA.bit.ENABLE = 1;
        while( TC0->COUNT32.STATUS.bit.SYNCBUSY )
            ;
        _isPaused = 0;
    }
}

void syncMicrosToRTC()
{
    uint32_t count = RTC_US_PER_COUNT * countRTC();
    count *= _cyclesPerUs;
    TC0->COUNT32.COUNT.reg = count;
    while( TC0->COUNT32.STATUS.bit.SYNCBUSY )
        ;
    resumeMicrosFromSleep();
}

uint32_t micros()
{
    uint32_t count = TC0->COUNT32.COUNT.reg;
    count /= _cyclesPerUs;
    _micros = ( secondsRTC() * 1000000UL ) + count;
    return _micros;
}

void delayMicroseconds( uint32_t us )
{
    uint32_t start = micros();
    do {
        yield();
    } while( ( micros() - start ) < us );
}