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
#include "sleep.h"

#define RTC_MAX_STEPS 0x1FFFFFFFFFFFF
#define RTC_WAIT_SYNC while( RTC->MODE1.STATUS.bit.SYNCBUSY )
#define RTC_SET_READS                     \
    {                                     \
        RTC->MODE1.READREQ.bit.RCONT = 1; \
        RTC->MODE1.READREQ.bit.RREQ = 1;  \
        RTC_WAIT_SYNC;                    \
    }

volatile uint64_t _rtcSec = 0;

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

uint64_t stepsRTC()
{
    uint16_t count;
    uint32_t ovf = 0;
    count = RTC->MODE1.COUNT.reg;
    // COUNT is a synchronized variable which may be stale. If it is
    // stale _rtcSec might already been incremented, but COUNT not yet
    // wrapped. We wait to the next increment of COUNT so we have a
    // current value.
    if( count > RTC_STEPS_PER_SEC - 8 ) {
        for( uint16_t ncount = count; ncount == count; ) {
            int prim = __get_PRIMASK();
            __disable_irq();
            count = RTC->MODE1.COUNT.reg;
            ovf = RTC->MODE1.INTFLAG.bit.OVF != 0;
            if( !prim ) __enable_irq();
        }
    }
    return ( ( _rtcSec + ovf ) << 15 ) | count;
}

uint64_t secondsRTC()
{
    return _rtcSec;
}

void delayRTCSteps( uint64_t steps )
{
    uint64_t start = stepsRTC();
    uint64_t rSteps = steps;
    uint64_t remaining =
        ( RTC_STEPS_PER_SEC - ( stepsRTC() & RTC_STEPS_OVERFLOW ) );
    do {
        // If we will be waiting long enough for an overflow interrupt to occur
        // go to sleep.
        if( remaining < rSteps ) {
            rSteps -= remaining;
            // If Serial is enabled sleep in Idle mode otherwise go into
            // Standby.
#if defined( __SAMD20E18__ )
            if( SERCOM3->USART.CTRLA.bit.ENABLE )
                sleepCPU( PM_SLEEP_IDLE_CPU_Val );
            else
                sleepCPU( PM_SLEEP_STANDBY_Val );
#endif /* __SAMD20E18__ */
            remaining = ( RTC_STEPS_PER_SEC - ( stepsRTC() & 0x7FFF ) );
        }
    } while( ( stepsRTC() - start ) < steps );
}

void RTC_IRQHandler()
{
    if( ( ++_rtcSec ) > RTC_MAX_STEPS ) _rtcSec = 0;
    RTC->MODE1.INTFLAG.bit.OVF = 1;
}
