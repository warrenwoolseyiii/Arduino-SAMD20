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

#include "RTC.h"
#include "clocks.h"
#include "sam.h"

#define RTC_WAIT_SYNC while( RTC->MODE1.STATUS.bit.SYNCBUSY )

volatile uint32_t _rtcSec = 0;

/* Initializes the RTC with a 32768 Hz input clock source. The resolution of the
 * RTC module is therefore 30.5 uS. The RTC interrupt is set to trigger an overflow
 * once a second which serves to wake the processor. */
void initRTC()
{
  initGenericClk( GCLK_CLKCTRL_GEN_GCLK1_Val, GCLK_CLKCTRL_ID_RTC_Val );
  disableRTC();

  RTC->MODE1.CTRL.bit.MODE = 1;
  RTC_WAIT_SYNC;
  RTC->MODE1.PER.reg = RTC_STEPS_PER_SEC - 1;
  RTC_WAIT_SYNC;
  RTC->MODE1.INTENSET.bit.OVF = 1;
  NVIC_EnableIRQ( RTC_IRQn );

  RTC->MODE1.CTRL.bit.ENABLE = 1;
  RTC_WAIT_SYNC;

  // Enable continuous read of the count register
  RTC->MODE1.READREQ.bit.RCONT = 1;
}

void disableRTC()
{
  RTC->MODE1.CTRL.bit.SWRST = 1;
  while( RTC->MODE1.STATUS.bit.SYNCBUSY || RTC->MODE1.CTRL.bit.SWRST );
  NVIC_DisableIRQ( RTC_IRQn );

  _rtcSec = 0;
}

uint32_t stepsRTC()
{
  uint32_t ticks = RTC->MODE1.COUNT.reg;
  return ( _rtcSec << 15 ) | ticks;
}

uint32_t secondsRTC()
{
  return _rtcSec;
}

void RTC_IRQHandler()
{
  // Due to millisRTC requiring this parameter to LSH 15 bits, the roll-over
  // must be handled before we LSH the MSB out of the _rtcSec value
  if( ( ++_rtcSec ) & 0x20000 ) _rtcSec = 0; 
  RTC->MODE1.INTFLAG.bit.OVF = 1;
}
