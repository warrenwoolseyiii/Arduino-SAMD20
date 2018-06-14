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

#define CC_8_BIT_MAX  0xFF
#define CC_16_BIT_MAX 0xFFFF
#define CC_32_BIT_MAX 0xFFFFFFFF

#define WAIT_TC_REGS_SYNC( x ) while( x->COUNT32.STATUS.bit.SYNCBUSY );

TimerCounter::TimerCounter( Tc* timerCounter, IRQn_Type irqNum )
{
  _timerCounter = timerCounter;
  _irqNum = irqNum;
  isrPtr = NULL;
}

void TimerCounter::registerISR( void (*isr)() )
{
  if( isr != NULL )
    isrPtr = isr;
}

void TimerCounter::deregisterISR()
{
  isrPtr = NULL;
}

void TimerCounter::begin( uint32_t frequency, int8_t outputPin, TCMode_t mode)
{
  end();

  _maxFreq = SystemCoreClock / 2;

  // Enable GCLK
  uint16_t gcmSrc = ( ( _irqNum == TC3_IRQn ) ?  GCM_TC2_TC3 : GCM_TC4_TC5 );
  GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID( gcmSrc ));
  while( GCLK->STATUS.bit.SYNCBUSY );

  // SWRST
  _timerCounter->COUNT32.CTRLA.reg = TC_CTRLA_SWRST;
  WAIT_TC_REGS_SYNC( _timerCounter )
  while ( _timerCounter->COUNT32.CTRLA.bit.SWRST );

  // Configure clock pre-scaler and compare capture value
  switch( mode ) {
    case tc_mode_8_bit: 
    _timerCounter->COUNT8.CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT8_Val;
    _timerCounter->COUNT8.CC[0].reg = (uint8_t)setDividerAndCC( frequency, CC_8_BIT_MAX );
    break;
    case tc_mode_16_bit: 
    _timerCounter->COUNT16.CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT16_Val;
    _timerCounter->COUNT16.CC[0].reg = (uint16_t)setDividerAndCC( frequency, CC_16_BIT_MAX );
    break;
    case tc_mode_32_bit: 
    _timerCounter->COUNT32.CTRLA.bit.MODE = TC_CTRLA_MODE_COUNT32_Val;
    _timerCounter->COUNT32.CC[0].reg = setDividerAndCC( frequency, CC_32_BIT_MAX );
    break;
    default:
      return;
  }
  WAIT_TC_REGS_SYNC( _timerCounter );

  _timerCounter->COUNT32.INTENSET.bit.MC0 = 1; // Enable compare capture interrupt 0

  // Enable the module and interrupts
  _timerCounter->COUNT32.CTRLA.bit.ENABLE = 1;
  WAIT_TC_REGS_SYNC( _timerCounter );
  NVIC_EnableIRQ( _irqNum );
}

void TimerCounter::end()
{
  // Clear IRQ
  NVIC_DisableIRQ( _irqNum );
  NVIC_ClearPendingIRQ( _irqNum );
  
  _timerCounter->COUNT32.CTRLA.bit.ENABLE = 0;
  WAIT_TC_REGS_SYNC( _timerCounter )
}

void TimerCounter::IrqHandler()
{
  _timerCounter->COUNT16.INTFLAG.bit.MC0 = 1;
  if( isrPtr != NULL )
    isrPtr();
}

uint32_t TimerCounter::setDividerAndCC( uint32_t freq, uint32_t maxCC )
{
  /* _timerCounter type Tc is a union between TcCount8, TcCount16, TcCount32. 
   * Each struct points to the same memory locations except for COUNT and CC.
   * See more about unions at any reputable source of C / C++ language documentation,
   * or in the definition in tc.h
   */
  uint32_t ccValue;

  ccValue = _maxFreq / freq - 1;
  _timerCounter->COUNT32.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV1;
  
  uint8_t i = 0;
  
  while( ccValue > maxCC ) {
    ccValue = _maxFreq / freq / ( 2 << i ) - 1;
    i++;
    if( i == 4 || i == 6 || i == 8 ) //DIV32 DIV128 and DIV512 are not available
      i++;
  }
  
  switch( i - 1 ) {
    case 0: _timerCounter->COUNT32.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV2_Val; break;
    case 1: _timerCounter->COUNT32.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV4_Val; break;
    case 2: _timerCounter->COUNT32.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV8_Val; break;
    case 3: _timerCounter->COUNT32.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV16_Val; break;
    case 5: _timerCounter->COUNT32.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV64_Val; break;
    case 7: _timerCounter->COUNT32.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV256_Val; break;
    case 9: _timerCounter->COUNT32.CTRLA.bit.PRESCALER = TC_CTRLA_PRESCALER_DIV1024_Val; break;
    default: break;
  }

  _timerCounter->COUNT32.CTRLA.bit.WAVEGEN = TC_CTRLA_WAVEGEN_MFRQ; // Toggle mode

  return ccValue;
}