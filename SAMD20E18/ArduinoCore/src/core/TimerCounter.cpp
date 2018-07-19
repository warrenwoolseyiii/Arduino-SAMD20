/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.

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

#include "TimerCounter.h"
#include <stddef.h>
#include "WVariant.h"
#include "wiring.h"

#define CC_8_BIT_MAX 0xFF
#define CC_16_BIT_MAX 0xFFFF
#define CC_32_BIT_MAX 0xFFFFFFFF

#define TIMER_NVIC_PRIORITY ( ( 1 << __NVIC_PRIO_BITS ) - 1 )
#define WAIT_TC_REGS_SYNC( x )              \
    while( x->COUNT16.STATUS.bit.SYNCBUSY ) \
        ;

TimerCounter::TimerCounter( Tc *timerCounter )
{
    _timerCounter = timerCounter;

    if( _timerCounter == TC0 ) {
        _APBCMask = PM_APBCMASK_TC0;
        _clkID = GCLK_CLKCTRL_ID_TC0_TC1_Val;
        _irqn = (uint32_t)TC0_IRQn;
    }
    else if( _timerCounter == TC1 ) {
        _APBCMask = PM_APBCMASK_TC1;
        _clkID = GCLK_CLKCTRL_ID_TC0_TC1_Val;
        _irqn = (uint32_t)TC1_IRQn;
    }
    else if( _timerCounter == TC2 ) {
        _APBCMask = PM_APBCMASK_TC2;
        _clkID = GCLK_CLKCTRL_ID_TC2_TC3_Val;
        _irqn = (uint32_t)TC2_IRQn;
    }
    else if( _timerCounter == TC3 ) {
        _APBCMask = PM_APBCMASK_TC3;
        _clkID = GCLK_CLKCTRL_ID_TC2_TC3_Val;
        _irqn = (uint32_t)TC3_IRQn;
    }
    else if( _timerCounter == TC4 ) {
        _APBCMask = PM_APBCMASK_TC4;
        _clkID = GCLK_CLKCTRL_ID_TC4_TC5_Val;
        _irqn = (uint32_t)TC4_IRQn;
    }
    else if( _timerCounter == TC5 ) {
        _APBCMask = PM_APBCMASK_TC5;
        _clkID = GCLK_CLKCTRL_ID_TC4_TC5_Val;
        _irqn = (uint32_t)TC5_IRQn;
    }
    else {
        _APBCMask = 0;
        _clkID = 0;
        _irqn = 0;
    }

    isrPtr = NULL;
    _mode = tc_mode_16_bit;
    _isPaused = false;
}

void TimerCounter::registerISR( void ( *isr )() )
{
    if( isr != NULL ) isrPtr = isr;
}

void TimerCounter::deregisterISR()
{
    isrPtr = NULL;
}

void TimerCounter::begin( uint32_t frequency, int8_t outputPin, TCMode_t mode,
                          bool useInterrupts )
{
    _maxFreq = SystemCoreClock / 2;
    _mode = mode;

    if( _clkID == 0 ) return;
    enableAPBCClk( _APBCMask, 1 );
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK0_Val, _clkID );
    if( useInterrupts ) NVIC_EnableIRQ( (IRQn_Type)_irqn );

    // SWRST
    reset();

    // Configure clock pre-scaler and compare capture value
    uint32_t ctrlABits = 0;
    switch( mode ) {
        case tc_mode_8_bit:
            ctrlABits |= TC_CTRLA_MODE_COUNT8;
            _timerCounter->COUNT8.CC[0].reg =
                (uint8_t)setDividerAndCC( frequency, CC_8_BIT_MAX, ctrlABits );
            break;
        case tc_mode_16_bit:
            ctrlABits |= TC_CTRLA_MODE_COUNT16;
            _timerCounter->COUNT16.CC[0].reg = (uint16_t)setDividerAndCC(
                frequency, CC_16_BIT_MAX, ctrlABits );
            break;
        case tc_mode_32_bit:
            ctrlABits |= TC_CTRLA_MODE_COUNT32;
            _timerCounter->COUNT16.CC[0].reg = (uint16_t)setDividerAndCC(
                frequency, CC_32_BIT_MAX, ctrlABits );
            break;
        default: return;
    }
    WAIT_TC_REGS_SYNC( _timerCounter )

    if( useInterrupts ) {
        _timerCounter->COUNT16.INTENSET.bit.MC0 =
            1; // Enable compare capture interrupt 0
    }

    // Allow continuous reads
    _timerCounter->COUNT16.READREQ.bit.RCONT = 1;

    // Enable the module and interrupts
    _timerCounter->COUNT16.CTRLA.bit.ENABLE = 1;
    WAIT_TC_REGS_SYNC( _timerCounter )
}

void TimerCounter::reset()
{
    _timerCounter->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
    while( _timerCounter->COUNT16.CTRLA.bit.SWRST )
        ;
    _isPaused = false;
}

void TimerCounter::end()
{
    reset();

    NVIC_DisableIRQ( (IRQn_Type)_irqn );
    disableGenericClk( _clkID );
    enableAPBCClk( _APBCMask, 0 );
}

void TimerCounter::resume()
{
    if( _isPaused ) {
        _timerCounter->COUNT16.CTRLA.bit.ENABLE = 1;
        WAIT_TC_REGS_SYNC( _timerCounter )
    }
}

void TimerCounter::pause()
{
    if( !_isPaused ) {
        _timerCounter->COUNT16.CTRLA.bit.ENABLE = 0;
        WAIT_TC_REGS_SYNC( _timerCounter )
        _isPaused = true;
    }
}

void TimerCounter::IrqHandler()
{
    _timerCounter->COUNT16.INTFLAG.bit.MC0 = 1;
    if( isrPtr != NULL ) isrPtr();
}

uint32_t TimerCounter::getCount()
{
    uint32_t count = 0;
    switch( _mode ) {
    case tc_mode_8_bit:
        count = _timerCounter->COUNT8.COUNT.reg;
        break;
    case tc_mode_16_bit:
        count = _timerCounter->COUNT16.COUNT.reg;
        break;
    case tc_mode_32_bit:
        count = _timerCounter->COUNT32.COUNT.reg;
        break;
    }
    
    return count;
}

void TimerCounter::setCount( uint32_t count )
{
    switch( _mode ) {
    case tc_mode_8_bit:
        _timerCounter->COUNT8.COUNT.reg = count & CC_8_BIT_MAX;
        break;
    case tc_mode_16_bit:
        _timerCounter->COUNT16.COUNT.reg = count & CC_16_BIT_MAX;
        break;
    case tc_mode_32_bit:
        _timerCounter->COUNT32.COUNT.reg = count;
        break;
    }

    WAIT_TC_REGS_SYNC( _timerCounter )
}

uint32_t TimerCounter::setDividerAndCC( uint32_t freq, uint16_t maxCC,
                                        uint32_t ctrlA )
{
    /* _timerCounter type Tc is a union between TcCount8, TcCount16, TcCount32.
     * Each struct points to the same memory locations except for COUNT and CC.
     * See more about unions at any reputable source of C / C++ language
     * documentation, or in the definition in tc.h
     */
    uint32_t ccValue;
    uint32_t preScaleBits = 0;

    ctrlA |= TC_CTRLA_WAVEGEN_MFRQ; // Toggle mode

    ccValue = _maxFreq / freq - 1;
    preScaleBits = TC_CTRLA_PRESCALER_DIV1;

    uint8_t i = 0;

    while( i <= 9 ) {
        ccValue = _maxFreq / freq / ( 2 << i ) - 1;
        if( ccValue < maxCC ) break;
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

    ctrlA |= preScaleBits;
    _timerCounter->COUNT16.CTRLA.reg |= ctrlA;
    WAIT_TC_REGS_SYNC( _timerCounter );

    return ccValue;
}