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

#include "TimerCounter.h"
#include "wiring_private.h"
#include "wiring_digital.h"
#include "clocks.h"
#include "WVariant.h"

#define CC_8_BIT_MAX 0xFF
#define CC_16_BIT_MAX 0xFFFF
#define CC_32_BIT_MAX 0xFFFFFFFF

#define TIMER_NVIC_PRIORITY ( ( 1 << __NVIC_PRIO_BITS ) - 1 )

TimerCounter::TimerCounter( Tc *timerCounter )
{
    _timerCounter = timerCounter;

    if( _timerCounter == TC2 ) {
        _APBCMask = PM_APBCMASK_TC2;
        _clkID = GCLK_CLKCTRL_ID_TC2_TC3_Val;
        _irqn = (uint32_t)TC2_IRQn;
        _tcNum = 2;
    }
    else if( _timerCounter == TC3 ) {
        _APBCMask = PM_APBCMASK_TC3;
        _clkID = GCLK_CLKCTRL_ID_TC2_TC3_Val;
        _irqn = (uint32_t)TC3_IRQn;
        _tcNum = 3;
    }
    else if( _timerCounter == TC4 ) {
        _APBCMask = PM_APBCMASK_TC4;
        _clkID = GCLK_CLKCTRL_ID_TC4_TC5_Val;
        _irqn = (uint32_t)TC4_IRQn;
        _tcNum = 4;
    }
    else if( _timerCounter == TC5 ) {
        _APBCMask = PM_APBCMASK_TC5;
        _clkID = GCLK_CLKCTRL_ID_TC4_TC5_Val;
        _irqn = (uint32_t)TC5_IRQn;
        _tcNum = 5;
    }
    else {
        _APBCMask = 0;
        _clkID = 0;
        _irqn = 0;
        _tcNum = -1;
    }

    isrPtr = NULL;
    _mode = tc_mode_16_bit;
    _ccVal = 0;
    _ctrlA = 0;
    _isPaused = false;
    _isActive = false;
}

void TimerCounter::registerISR( void ( *isr )() )
{
    if( isr != NULL ) isrPtr = isr;
}

void TimerCounter::deregisterISR()
{
    isrPtr = NULL;
}

void TimerCounter::beginPWM( uint32_t frequency, uint8_t dutyCycle )
{
    begin( frequency / 2, false, tc_mode_16_bit, false );
    pause();

    // Set wave generation
    _timerCounter->COUNT16.CTRLA.bit.WAVEGEN = TC_CTRLA_WAVEGEN_MPWM_Val;
    waitRegSync();

    // Duty cycle
    setPWMDutyCycle( dutyCycle );

    switch( _tcNum ) {
        case 2: pinPeripheral( 6, PIO_TIMER_ALT ); break;
        case 3: pinPeripheral( 4, PIO_TIMER ); break;
        case 4: pinPeripheral( 10, PIO_TIMER_ALT ); break;
        case 5: pinPeripheral( 0, PIO_TIMER_ALT ); break;
    }

    resume();
}

void TimerCounter::begin( uint32_t frequency, bool output, TCMode_t mode,
                          bool useInterrupts )
{
    _maxFreq = SystemCoreClock / 2;
    _mode = mode;

    if( _clkID == 0 ) return;
    enableAPBCClk( _APBCMask, 1 );
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK0_Val, _clkID );
    if( useInterrupts ) NVIC_EnableIRQ( (IRQn_Type)_irqn );
    if( output ) {
        switch( _tcNum ) {
            case 2: pinPeripheral( 5, PIO_TIMER_ALT ); break;
            case 3: pinPeripheral( 3, PIO_TIMER ); break;
            case 4: pinPeripheral( 9, PIO_TIMER_ALT ); break;
            case 5: pinPeripheral( 1, PIO_TIMER_ALT ); break;
        }
    }

    // SWRST
    reset();

    switch( mode ) {
        case tc_mode_8_bit:
            _ctrlA = TC_CTRLA_MODE_COUNT8;

            // Configure period, and pre-scalers
            setDividerAndCC( frequency, CC_8_BIT_MAX );
            _timerCounter->COUNT8.CC[0].reg = (uint8_t)_ccVal;
            waitRegSync();

            // Enable compare capture interrupt 0
            if( useInterrupts ) _timerCounter->COUNT8.INTENSET.bit.MC0 = 1;

            // Allow continuous reads
            _timerCounter->COUNT8.READREQ.bit.RCONT = 1;

            // Enable the module and interrupts
            _ctrlA |= TC_CTRLA_ENABLE;
            _timerCounter->COUNT8.CTRLA.reg = _ctrlA;
            waitRegSync();

            break;
        case tc_mode_16_bit:
            _ctrlA = TC_CTRLA_MODE_COUNT16;

            // Configure period, and pre-scalers
            setDividerAndCC( frequency, CC_16_BIT_MAX );
            _timerCounter->COUNT16.CC[0].reg = (uint16_t)_ccVal;
            waitRegSync();

            // Enable compare capture interrupt 0
            if( useInterrupts ) _timerCounter->COUNT16.INTENSET.bit.MC0 = 1;

            // Allow continuous reads
            _timerCounter->COUNT16.READREQ.bit.RCONT = 1;

            // Enable the module and interrupts
            _ctrlA |= TC_CTRLA_ENABLE;
            _timerCounter->COUNT16.CTRLA.reg = _ctrlA;
            waitRegSync();

            break;
        case tc_mode_32_bit:
            if( ( _tcNum % 2 ) != 0 ) return;

            _ctrlA = TC_CTRLA_MODE_COUNT32;

            // Configure period, and pre-scalers
            setDividerAndCC( frequency, CC_32_BIT_MAX );
            _timerCounter->COUNT32.CC[0].reg = (uint8_t)_ccVal;
            waitRegSync();

            // Enable compare capture interrupt 0
            if( useInterrupts ) _timerCounter->COUNT32.INTENSET.bit.MC0 = 1;

            // Allow continuous reads
            _timerCounter->COUNT32.READREQ.bit.RCONT = 1;

            // Enable the module and interrupts
            _ctrlA |= TC_CTRLA_ENABLE;
            _timerCounter->COUNT32.CTRLA.reg = _ctrlA;
            waitRegSync();

            break;
        default: return;
    }

    _isActive = true;
}

void TimerCounter::reset()
{
    switch( _mode ) {
        case tc_mode_8_bit:
            _timerCounter->COUNT8.CTRLA.reg = TC_CTRLA_SWRST;
            while( _timerCounter->COUNT8.CTRLA.bit.SWRST )
                ;
            break;
        case tc_mode_16_bit:
            _timerCounter->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
            while( _timerCounter->COUNT16.CTRLA.bit.SWRST )
                ;
            break;
        case tc_mode_32_bit:
            if( ( _tcNum % 2 ) == 0 ) {
                _timerCounter->COUNT32.CTRLA.reg = TC_CTRLA_SWRST;
                while( _timerCounter->COUNT32.CTRLA.bit.SWRST )
                    ;
            }
            break;
    }

    _ctrlA = 0;
    _ccVal = 0;
    _isPaused = false;
}

void TimerCounter::end()
{
    reset();
    _isActive = false;

    switch( _tcNum ) {
        case 2: pinMode( 5, TRI_STATE ); break;
        case 3: pinMode( 3, TRI_STATE ); break;
        case 4: pinMode( 9, TRI_STATE ); break;
        case 5: pinMode( 1, TRI_STATE ); break;
    }

    NVIC_DisableIRQ( (IRQn_Type)_irqn );
    disableGenericClk( _clkID );
    enableAPBCClk( _APBCMask, 0 );
}

void TimerCounter::resume()
{
    if( _isPaused ) {
        _isPaused = false;
        switch( _mode ) {
            case tc_mode_8_bit:
                _timerCounter->COUNT8.CTRLA.bit.ENABLE = 1;
                break;
            case tc_mode_16_bit:
                _timerCounter->COUNT16.CTRLA.bit.ENABLE = 1;
                break;
            case tc_mode_32_bit:
                if( ( _tcNum % 2 ) == 0 )
                    _timerCounter->COUNT32.CTRLA.bit.ENABLE = 1;
                break;
        }

        waitRegSync();
    }
}

void TimerCounter::pause()
{
    if( !_isPaused ) {
        _isPaused = true;
        switch( _mode ) {
            case tc_mode_8_bit:
                _timerCounter->COUNT8.CTRLA.bit.ENABLE = 0;
                break;
            case tc_mode_16_bit:
                _timerCounter->COUNT16.CTRLA.bit.ENABLE = 0;
                break;
            case tc_mode_32_bit:
                if( ( _tcNum % 2 ) == 0 )
                    _timerCounter->COUNT32.CTRLA.bit.ENABLE = 0;
                break;
        }

        waitRegSync();
    }
}

void TimerCounter::IrqHandler()
{
    switch( _mode ) {
        case tc_mode_8_bit: _timerCounter->COUNT8.INTFLAG.bit.MC0 = 1; break;
        case tc_mode_16_bit: _timerCounter->COUNT16.INTFLAG.bit.MC0 = 1; break;
        case tc_mode_32_bit: _timerCounter->COUNT32.INTFLAG.bit.MC0 = 1; break;
    }

    if( isrPtr != NULL ) isrPtr();
}

uint32_t TimerCounter::getCount()
{
    uint32_t count = 0;
    switch( _mode ) {
        case tc_mode_8_bit: count = _timerCounter->COUNT8.COUNT.reg; break;
        case tc_mode_16_bit: count = _timerCounter->COUNT16.COUNT.reg; break;
        case tc_mode_32_bit: count = _timerCounter->COUNT32.COUNT.reg; break;
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
        case tc_mode_32_bit: _timerCounter->COUNT32.COUNT.reg = count; break;
    }

    waitRegSync();
}

void TimerCounter::setPWMDutyCycle( uint8_t dutyCycle )
{
    // Set duty cycle
    uint32_t cc0 = _timerCounter->COUNT16.CC[0].reg;
    uint32_t cc1 = ( ( cc0 << 8 ) * dutyCycle ) / 100;
    cc1 >>= 8;
    _timerCounter->COUNT16.CC[1].reg = cc1;
    waitRegSync();
}

void TimerCounter::setDividerAndCC( uint32_t freq, uint32_t maxCC )
{
    uint32_t preScaleBits = 0;

    _ctrlA |= TC_CTRLA_WAVEGEN_MFRQ; // Toggle mode

    _ccVal = _maxFreq / freq - 1;
    preScaleBits = TC_CTRLA_PRESCALER_DIV1;

    uint8_t i = 0;

    while( i <= 9 ) {
        _ccVal = _maxFreq / freq / ( 2 << i ) - 1;
        if( _ccVal < maxCC ) break;
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

    _ctrlA |= preScaleBits;
}

void TimerCounter::waitRegSync()
{
    switch( _mode ) {
        case tc_mode_8_bit:
            while( _timerCounter->COUNT8.STATUS.bit.SYNCBUSY )
                ;
            break;
        case tc_mode_16_bit:
            while( _timerCounter->COUNT16.STATUS.bit.SYNCBUSY )
                ;
            break;
        case tc_mode_32_bit:
            if( ( _tcNum % 2 ) == 0 )
                while( _timerCounter->COUNT32.STATUS.bit.SYNCBUSY )
                    ;
            break;
    }
}
