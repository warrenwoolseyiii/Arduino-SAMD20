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

#include <sam.h>
#include "RTC.h"
#include "clocks.h"
#include "micros.h"

#define RTC_MAX_STEPS 0x40000000000
#define RTC_WAIT_SYNC while( RTC->MODE1.STATUS.bit.SYNCBUSY )
#define RTC_SET_READS                     \
    {                                     \
        RTC->MODE1.READREQ.bit.RCONT = 1; \
        RTC->MODE1.READREQ.bit.RREQ = 1;  \
        RTC_WAIT_SYNC;                    \
    }

volatile int64_t _rtcSec = 0;

#define RTC_STEPS ( ( _rtcSec << 15 ) | RTC->MODE1.COUNT.reg )

/* Initializes the RTC with a 32768 Hz input clock source. The resolution of the
 * RTC module is therefore 30.5 uS. The RTC interrupt is set to trigger an
 * overflow once a second which serves to wake the processor. */
void initRTC()
{
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK1_Val, GCLK_CLKCTRL_ID_RTC_Val );
    enableAPBAClk( PM_APBAMASK_RTC, 1 );
    RTC->MODE1.CTRL.bit.SWRST = 1;
    while( RTC->MODE1.STATUS.bit.SYNCBUSY || RTC->MODE1.CTRL.bit.SWRST )
        ;

    RTC->MODE1.CTRL.bit.MODE = 1;
    RTC_WAIT_SYNC;
    RTC->MODE1.PER.reg = RTC_STEPS_PER_SEC - 1;
    RTC_WAIT_SYNC;
    RTC->MODE1.INTENSET.bit.OVF = 1;
    NVIC_EnableIRQ( RTC_IRQn );

    RTC->MODE1.CTRL.bit.ENABLE = 1;
    RTC_WAIT_SYNC;

    // Enable continuous read of the count register
    RTC_SET_READS;
}

void disableRTC()
{
    RTC->MODE1.CTRL.bit.SWRST = 1;
    while( RTC->MODE1.STATUS.bit.SYNCBUSY || RTC->MODE1.CTRL.bit.SWRST )
        ;
    NVIC_DisableIRQ( RTC_IRQn );
    enableAPBAClk( PM_APBAMASK_RTC, 0 );
    disableGenericClk( GCLK_CLKCTRL_ID_RTC_Val );
    _rtcSec = 0;
}

volatile int64_t stepsRTC()
{
    return RTC_STEPS;
}

volatile int64_t secondsRTC()
{
    return _rtcSec;
}

void delayRTCSteps( int64_t steps )
{
    int64_t start = RTC_STEPS;
    while( ( RTC_STEPS - start ) < steps )
        ;
}

/* When the RTC rolls over we need to ensure the COUNT register is reset to zero
 * AND that it reads as such. The SAMD20 uses a bus synchronization scheme to
 * allow continuous reading of the COUNT register. However, during tight loops
 * such as a delay scheme or a while( millis() ) loop, the synchronization may
 * not happen every RTC tick. This can cause some serious underflow, overflow
 * problems during looping. In order to prevent this, on the overflow interrupt
 * we force the COUNT register to 0, then we force the read setup to ensure that
 * the bus has synchronized during the rollover */
volatile uint32_t _forceRead = 0;

void RTC_IRQHandler()
{
    // Force the COUNT register to 0
    RTC_WAIT_SYNC;
    RTC->MODE1.COUNT.reg = 0;

    // Due to millisRTC requiring this parameter to LSH 15 bits, the roll-over
    // must be handled before we LSH the MSB out of the _rtcSec value
    if( ( ++_rtcSec ) & RTC_MAX_STEPS ) _rtcSec = 0;
    RTC->MODE1.INTFLAG.bit.OVF = 1;

    // Force the bus to synchronize
    RTC_SET_READS;
    _forceRead = RTC->MODE1.COUNT.reg;
}
