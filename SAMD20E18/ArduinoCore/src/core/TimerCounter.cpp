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

#define WAIT_TC_REGS_SYNC( x ) while( x->COUNT16.STATUS.bit.SYNCBUSY );

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
  _timerCounter->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  WAIT_TC_REGS_SYNC( _timerCounter )
  while ( _timerCounter->COUNT16.CTRLA.bit.SWRST );

  // Configure clock pre-scaler and compare capture value
  uint32_t ctrlABits = 0;
  switch( mode ) {
    case tc_mode_8_bit: 
    ctrlABits |= TC_CTRLA_MODE_COUNT8;
    _timerCounter->COUNT8.CC[0].reg = (uint8_t)setDividerAndCC( frequency, CC_8_BIT_MAX, ctrlABits );
    break;
    case tc_mode_16_bit: 
    ctrlABits |= TC_CTRLA_MODE_COUNT16;
    _timerCounter->COUNT16.CC[0].reg = (uint16_t)setDividerAndCC( frequency, CC_16_BIT_MAX, ctrlABits );
    break;
    default:
      return;
  }
  WAIT_TC_REGS_SYNC( _timerCounter )

  _timerCounter->COUNT16.INTENSET.bit.MC0 = 1; // Enable compare capture interrupt 0

  // Enable the module and interrupts
  _timerCounter->COUNT16.CTRLA.bit.ENABLE = 1;
  WAIT_TC_REGS_SYNC( _timerCounter )
  NVIC_EnableIRQ( _irqNum );
}

void TimerCounter::end()
{
  // Clear IRQ
  NVIC_DisableIRQ( _irqNum );
  NVIC_ClearPendingIRQ( _irqNum );
  
  _timerCounter->COUNT16.CTRLA.bit.ENABLE = 0;
  WAIT_TC_REGS_SYNC( _timerCounter )
}

void TimerCounter::IrqHandler()
{
  _timerCounter->COUNT16.INTFLAG.bit.MC0 = 1;
  if( isrPtr != NULL )
    isrPtr();
}

uint32_t TimerCounter::setDividerAndCC( uint32_t freq, uint16_t maxCC, uint32_t ctrlA )
{
  /* _timerCounter type Tc is a union between TcCount8, TcCount16, TcCount32. 
   * Each struct points to the same memory locations except for COUNT and CC.
   * See more about unions at any reputable source of C / C++ language documentation,
   * or in the definition in tc.h
   */
  uint32_t ccValue;
  uint32_t preScaleBits = 0;

  ctrlA |= TC_CTRLA_WAVEGEN_MFRQ; // Toggle mode
  
  ccValue = _maxFreq / freq - 1;
  preScaleBits = TC_CTRLA_PRESCALER_DIV1;
  
  uint8_t i = 0;
  
  while( i <= 9 ) {
    ccValue = _maxFreq / freq / ( 2 << i ) - 1;
    if( ccValue < maxCC )
      break;
    i++;
    if( i == 4 || i == 6 || i == 8 ) //DIV32 DIV128 and DIV512 are not available
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