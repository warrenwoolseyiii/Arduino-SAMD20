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

#include "delay.h"
#include "RTC.h"
#include "SysTick.h"
#include "atomic.h"

volatile Micros_Debug_t      _microsDebug = {0, 0, 0};
volatile DelayMicros_Debug_t _delayMicrosDebug = {0, 0};

void getMicrosDebugInfo( Micros_Debug_t *mic, DelayMicros_Debug_t *dMic )
{
    mic->rtnMicros = _microsDebug.rtnMicros;
    mic->tix = _microsDebug.tix;
    mic->inside = _microsDebug.inside;

    dMic->mic = _delayMicrosDebug.mic;
    dMic->inside = _delayMicrosDebug.inside;
}

uint32_t millis()
{
    uint64_t steps = stepsRTC();
    return ( uint32_t )( RTC_EXACT_STEPS_TO_MILLIS( steps ) & 0xFFFFFFFF );
}

void delay( uint32_t ms )
{
    delayRTCSteps( RTC_EXACT_MILLIS_TO_STEPS( (int64_t)ms ) );
}

uint32_t micros()
{
    _microsDebug.inside = 1;
    _microsDebug.tix = getCPUTicks();

    if( SystemCoreClock > 8000000ul )
        _microsDebug.rtnMicros =
            ( uint32_t )( ( _microsDebug.tix / 48 ) & 0xFFFFFFFF );
    else
        _microsDebug.rtnMicros = ( uint32_t )(
            ( _microsDebug.tix >> ( SystemCoreClock >> 21 ) ) & 0xFFFFFFFF );

    _microsDebug.inside = 0;
    return _microsDebug.rtnMicros;
}

void delayMicroseconds( uint32_t us )
{
    _delayMicrosDebug.inside = 1;
    _delayMicrosDebug.mic = (uint64_t)us;

    if( SystemCoreClock > 8000000ul )
        delayCPUTicks( _delayMicrosDebug.mic * 48 );
    else
        delayCPUTicks( _delayMicrosDebug.mic << ( SystemCoreClock >> 21 ) );

    _delayMicrosDebug.inside = 0;
}
