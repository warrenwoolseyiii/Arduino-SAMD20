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
#include "atomic.h"

// Minimum step length in delay for us to sleep during the delay
#define RTC_MIN_DELAY_LEN_TO_SLEEP 32

// Approximate amount of steps required to enable continuous reads
#define RTC_CNT_READ_ENABLE_DELAY 10

#define RTC_MAX_STEPS 0x1FFFFFFFFFFFF
#define RTC_SYNC_BUSY ( RTC->MODE1.STATUS.bit.SYNCBUSY )
#define RTC_WAIT_SYNC while( RTC_SYNC_BUSY )
#define RTC_SET_READS                          \
    {                                          \
        ATOMIC_OPERATION( {                    \
            if( RTC_SYNC_BUSY ) RTC_WAIT_SYNC; \
            RTC->MODE1.READREQ.bit.RCONT = 1;  \
            RTC->MODE1.READREQ.bit.RREQ = 1;   \
        } )                                    \
    }

void RTC_IRQHandler();

volatile uint8_t               _insideDelaySetup = 0;
volatile uint64_t              _rtcOverFlows;
volatile RTCSteps_Debug_t      _stepsDebug = {0, 0, 0, 0, 0};
volatile DelayRTCSteps_Debug_t _delayStepsDebug = {0, 0, 0, 0, 0, 0};
volatile RTCIRQ_Debug_t        _rtcIrqDebug = {0, 0, 0, 0};
void ( *userOverFlowISR )() = 0;
int ( *userIdleTaskHasWork )() = 0;

// Initializes the RTC with a 32768 Hz input clock source. The resolution of the
//  RTC module is therefore 30.5 uS. The RTC overflow interrupt is set to
//  trigger once ever 32768 clock cycles which is one overflow per second.
void initRTC()
{
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK1_Val, GCLK_CLKCTRL_ID_RTC_Val );
    enableAPBAClk( PM_APBAMASK_RTC, 1 );

    ATOMIC_OPERATION( {
        if( RTC_SYNC_BUSY ) RTC_WAIT_SYNC;
        RTC->MODE1.CTRL.bit.SWRST = 1;
    } );
    while( RTC->MODE1.CTRL.bit.SWRST )
        ;

    ATOMIC_OPERATION( {
        if( RTC_SYNC_BUSY ) RTC_WAIT_SYNC;
        RTC->MODE1.CTRL.bit.MODE = 1;
    } )
    ATOMIC_OPERATION( {
        if( RTC_SYNC_BUSY ) RTC_WAIT_SYNC;
        RTC->MODE1.PER.reg = RTC_STEPS_OVERFLOW;
    } )

    RTC->MODE1.INTENSET.bit.OVF = 1;
    NVIC_EnableIRQ( RTC_IRQn );

    ATOMIC_OPERATION( {
        if( RTC_SYNC_BUSY ) RTC_WAIT_SYNC;
        RTC->MODE1.CTRL.bit.ENABLE = 1;
    } )

    // Enable continuous read of the count register
    RTC_SET_READS;
}

void disableRTC()
{
    ATOMIC_OPERATION( {
        if( RTC_SYNC_BUSY ) RTC_WAIT_SYNC;
        RTC->MODE1.CTRL.bit.SWRST = 1;
    } )
    while( RTC->MODE1.CTRL.bit.SWRST )
        ;

    NVIC_DisableIRQ( RTC_IRQn );
    enableAPBAClk( PM_APBAMASK_RTC, 0 );
    disableGenericClk( GCLK_CLKCTRL_ID_RTC_Val );
    _rtcOverFlows = 0;
}

uint64_t stepsRTC()
{
    _stepsDebug.inside = 1;
    _stepsDebug.overFlowCalled = 0;

    // If we are inside delayRTCSteps setup we need to enable continuous reads
    if( _insideDelaySetup ) {
        RTC_SET_READS
    }

    // If we are within 6 RTC steps of the overflow value we need to wait until
    // the overflow is complete to avoid a stale register and overflow count
    // value
    do {

        // Get the count and interrupt flag register values
        _stepsDebug.countReg = RTC->MODE1.COUNT.reg;
        _stepsDebug.rtcFlags = RTC->MODE1.INTFLAG.reg;

        // Check it the overflow interrupt was triggered and call it
        if( _stepsDebug.rtcFlags & RTC_MODE1_INTFLAG_OVF ) {
            RTC_SET_READS
            RTC_IRQHandler();
            _stepsDebug.overFlowCalled = 1;
            continue;
        }
    } while( _stepsDebug.countReg >= ( RTC_STEPS_OVERFLOW - 6 ) );

    // Return the total overflows with the current count register value
    _stepsDebug.rtnSteps = ( _rtcOverFlows << 15 ) | _stepsDebug.countReg;
    _stepsDebug.inside = 0;
    return _stepsDebug.rtnSteps;
}

uint64_t secondsRTC()
{
    return _rtcOverFlows;
}

void delayRTCSteps( uint64_t steps )
{
    delayRTCStepsIdle( steps, 0 );
}

void delayRTCStepsIdle( uint64_t steps, void ( *idleFunc )() )
{
    // Set the debug values to the step length to conserve the values
    _delayStepsDebug.inside = 1;
    _delayStepsDebug.steps = steps;
    _delayStepsDebug.sleepCounter = 0;

    // Set the start and cnt values to current steps
    _delayStepsDebug.cnt = _delayStepsDebug.start = stepsRTC();

    // Remain here while the delta between cnt and start is less than steps
    while( ( _delayStepsDebug.cnt - _delayStepsDebug.start ) <
           _delayStepsDebug.steps ) {

        // Check to see if the idle task can be called
        if( userIdleTaskHasWork && idleFunc ) {

            // Only call the idle task if we have a delay longer than 1 ms, and
            // if we have actual work to do
            if( userIdleTaskHasWork() &&
                ( _delayStepsDebug.steps -
                      ( _delayStepsDebug.cnt - _delayStepsDebug.start ) >
                  RTC_MIN_DELAY_LEN_TO_SLEEP ) ) {
                idleFunc();
                _delayStepsDebug.cnt = stepsRTC();
                continue;
            }
        }

        // Only attempt to sleep if interrupts are enabled
        if( !__get_PRIMASK() && NVIC_GetEnableIRQ( RTC_IRQn ) ) {

            // Check to see if the delay length is long enough for us to sleep
            _delayStepsDebug.internalDelay = ( int64_t )(
                ( _delayStepsDebug.start + _delayStepsDebug.steps ) -
                _delayStepsDebug.cnt );

            // If it is, lets go to sleep
            if( _delayStepsDebug.internalDelay > RTC_MIN_DELAY_LEN_TO_SLEEP ) {

                // Set the soft lock
                _insideDelaySetup = 1;
                _delayStepsDebug.sleepCounter++;

                // Clear the compare interrupt flag
                RTC->MODE1.INTFLAG.reg = RTC_MODE1_INTFLAG_CMP0;

                // Account for the ~10 RTC cycles it takes to re-enable reads at
                // the end of this section
                ATOMIC_OPERATION( {
                    if( RTC_SYNC_BUSY ) RTC_WAIT_SYNC;
                    RTC->MODE1.COMP[0].reg = ( uint16_t )(
                        ( ( _delayStepsDebug.internalDelay -
                            RTC_CNT_READ_ENABLE_DELAY ) +
                          ( _delayStepsDebug.cnt & RTC_STEPS_OVERFLOW ) ) &
                        RTC_STEPS_OVERFLOW );
                } )

                // Enable the compare interrupt
                RTC->MODE1.INTENSET.bit.CMP0 = 1;

                // Sleep the CPU
                sleepCPU( _deep_sleep );

                // Disable the compare interrupt, keep the overflow interrupt
                // enabled
                uint32_t b = RTC->MODE1.INTENSET.reg;
                if( b & RTC_MODE1_INTENSET_CMP0 ) {
                    b &= ~RTC_MODE1_INTENCLR_OVF;
                    RTC->MODE1.INTENCLR.reg = b;
                }

                // Writing to the compare register clears automatic reads,
                // re-enable automatic reads. Takes ~10 RTC cycles
                RTC_SET_READS

                // Release the soft lock
                _insideDelaySetup = 0;
            }
        }

        // Grab the new count coming out of sleep
        _delayStepsDebug.cnt = stepsRTC();
    }

    _delayStepsDebug.inside = 0;
}

void registerOverflowISR( void ( *ISRFunc )() )
{
    userOverFlowISR = ISRFunc;
}

void registerIdleTaskHasWork( int ( *Func )() )
{
    userIdleTaskHasWork = Func;
}

void getRTCDebugInfo( RTCSteps_Debug_t *steps, DelayRTCSteps_Debug_t *dSteps,
                      RTCIRQ_Debug_t *irqSteps )
{
    steps->countReg = _stepsDebug.countReg;
    steps->inside = _stepsDebug.inside;
    steps->overFlowCalled = _stepsDebug.overFlowCalled;
    steps->rtcFlags = _stepsDebug.rtcFlags;
    steps->rtnSteps = _stepsDebug.rtnSteps;

    dSteps->cnt = _delayStepsDebug.cnt;
    dSteps->inside = _delayStepsDebug.inside;
    dSteps->internalDelay = _delayStepsDebug.internalDelay;
    dSteps->sleepCounter = _delayStepsDebug.sleepCounter;
    dSteps->start = _delayStepsDebug.start;
    dSteps->steps = _delayStepsDebug.steps;

    irqSteps->inside = _rtcIrqDebug.inside;
    irqSteps->irqFlags = _rtcIrqDebug.irqFlags;
    irqSteps->maxOVF = _rtcIrqDebug.maxOVF;
    irqSteps->userISRCalled = _rtcIrqDebug.userISRCalled;
}

void RTC_IRQHandler()
{
    _rtcIrqDebug.inside = 1;
    _rtcIrqDebug.irqFlags = RTC->MODE1.INTFLAG.reg;
    _rtcIrqDebug.maxOVF = 0;
    _rtcIrqDebug.userISRCalled = 0;

    // RTC can wake the processor, ensure we exit sleep properly
    exitSleep();

    // Handle an overflow interrupt, call the user enabled handler if there is
    // one
    if( _rtcIrqDebug.irqFlags & RTC_MODE1_INTFLAG_OVF ) {

        if( ( ++_rtcOverFlows ) > RTC_MAX_STEPS ) {
            _rtcIrqDebug.maxOVF = 1;
            _rtcOverFlows = 0;
        }

        if( userOverFlowISR != 0 ) {
            _rtcIrqDebug.userISRCalled = 1;
            userOverFlowISR();
        }
    }

    RTC->MODE1.INTFLAG.reg = _rtcIrqDebug.irqFlags;
    _rtcIrqDebug.inside = 0;
}
