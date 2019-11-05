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

void RTC_IRQHandler();

volatile uint8_t  _insideDelaySetup = 0;
volatile uint64_t _rtcOverFlows;
void ( *userOverFlowISR )() = 0;

// Initializes the RTC with a 32768 Hz input clock source. The resolution of the
//  RTC module is therefore 30.5 uS. The RTC overflow interrupt is set to
//  trigger once ever 32768 clock cycles which is one overflow per second.
void initRTC()
{
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK1_Val, GCLK_CLKCTRL_ID_RTC_Val );
    enableAPBAClk( PM_APBAMASK_RTC, 1 );
    RTC->MODE1.CTRL.bit.SWRST = 1;
    while( RTC->MODE1.STATUS.bit.SYNCBUSY || RTC->MODE1.CTRL.bit.SWRST )
        ;

    RTC->MODE1.CTRL.bit.MODE = 1;
    RTC_WAIT_SYNC;
    RTC->MODE1.PER.reg = RTC_STEPS_OVERFLOW;
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
    _rtcOverFlows = 0;
}

uint64_t stepsRTC()
{
    uint64_t steps;
    uint32_t flags;

    if( _insideDelaySetup ) {
        RTC_SET_READS
    }

    do {
        steps = RTC->MODE1.COUNT.reg;
        flags = RTC->MODE1.INTFLAG.reg;
        if( flags & RTC_MODE1_INTFLAG_OVF ) {
            RTC_SET_READS
            RTC_IRQHandler();
            continue;
        }
    } while( steps >= ( RTC_STEPS_OVERFLOW - 6 ) );
    steps |= ( _rtcOverFlows << 15 );
    return steps;
}

uint64_t secondsRTC()
{
    return _rtcOverFlows;
}

// Debug struct
#if defined( _DEBUG_RTC_ )
volatile RTCDebugStuff_t rtcDBG;
#endif /* _DEBUG_RTC_ */

void delayRTCSteps( uint64_t steps )
{
    uint64_t start, cnt;
    cnt = start = stepsRTC();

    // Debug values
#if defined( _DEBUG_RTC_ )
    rtcDBG.start = start;
    rtcDBG.delta = steps;
    rtcDBG.ovfNdx = rtcDBG.compNdx = 0;
    rtcDBG.comp = rtcDBG.ovf = rtcDBG.isrNdx = rtcDBG.enNdx = 0;
#endif /* _DEBUG_RTC_ */

    while( ( cnt - start ) < steps ) {

        // Only attempt to sleep if interrupts are enabled
        if( !__get_PRIMASK() ) {
            int64_t d = ( int64_t )( ( start + steps ) - cnt );
            if( d > 32 ) {
                _insideDelaySetup = 1;

                // Clear the compare interrupt flag
                RTC->MODE1.INTFLAG.reg = RTC_MODE1_INTFLAG_CMP0;

                // Account for the ~10 RTC cycles it takes to re-enable reads at
                // the end of this section
                RTC->MODE1.COMP[0].reg = ( uint16_t )(
                    ( ( d - 10 ) + ( cnt & RTC_STEPS_OVERFLOW ) ) &
                    RTC_STEPS_OVERFLOW );
                RTC_WAIT_SYNC;
                RTC->MODE1.INTENSET.bit.CMP0 = 1;

                // Debug values
#if defined( _DEBUG_RTC_ )
                rtcDBG.compStepStart[rtcDBG.compNdx % 128] = cnt;
                rtcDBG.comp = 1;
#endif /* _DEBUG_RTC_ */

                sleepCPU( _deep_sleep );

                // Clear the compare interrupt enable bit
                uint32_t b = RTC->MODE1.INTENSET.reg;
                if( b & RTC_MODE1_INTENSET_CMP0 ) {
                    b &= ~RTC_MODE1_INTENCLR_OVF;
                    RTC->MODE1.INTENCLR.reg = b;
                }

                // Writing to the compare register clears automatic reads,
                // re-enable automatic reads. Takes ~10 RTC cycles
                RTC_SET_READS;

                _insideDelaySetup = 0;
            }
        }

        // Grab the new count coming out of sleep
        cnt = stepsRTC();

        // Debug values
#if defined( _DEBUG_RTC_ )
        if( rtcDBG.comp ) {
            rtcDBG.compStepEnd[rtcDBG.compNdx % 128] = cnt;
            rtcDBG.compNdx++;
        }
        if( rtcDBG.ovf ) {
            rtcDBG.ovfStepEnd[rtcDBG.ovfNdx % 128] = cnt;
            rtcDBG.ovfNdx++;
        }
        rtcDBG.comp = rtcDBG.ovf = 0;
#endif /* _DEBUG_RTC_ */
    }

#if defined( _DEBUG_RTC_ )
    rtcDBG.end = cnt;
    rtcDBG.delta = rtcDBG.end - rtcDBG.start;
#endif /* _DEBUG_RTC_ */
}

void registerOverflowISR( void ( *ISRFunc )() )
{
    userOverFlowISR = ISRFunc;
}

void RTC_IRQHandler()
{
    uint32_t flags = RTC->MODE1.INTFLAG.reg;

    // Debug values
#if defined( _DEBUG_RTC_ )
    uint32_t b = RTC->MODE1.INTENSET.reg;
    if( rtcDBG.comp || rtcDBG.ovf ) {
        rtcDBG.isrFlags[rtcDBG.isrNdx++ % 128] = flags;
        rtcDBG.enFlags[rtcDBG.enNdx++ % 128] = b;
    }
#endif /* _DEBUG_RTC_ */

    if( flags & RTC_MODE1_INTFLAG_OVF ) {
        if( ( ++_rtcOverFlows ) > RTC_MAX_STEPS ) _rtcOverFlows = 0;

        if( userOverFlowISR != 0 ) userOverFlowISR();
    }

    RTC->MODE1.INTFLAG.reg = flags;
}
