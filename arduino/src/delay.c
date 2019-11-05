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
    uint64_t tix;
    GET_CPU_TICKS( tix )
    if( SystemCoreClock > 8000000ul )
        return ( uint32_t )( ( tix / 48 ) & 0xFFFFFFFF );
    else
        return ( uint32_t )( ( tix >> ( SystemCoreClock >> 21 ) ) &
                             0xFFFFFFFF );
}

void delayMicroseconds( uint32_t us )
{
    uint64_t duration = us;
    if( SystemCoreClock > 8000000ul ) {
        DELAY_CPU_TICKS( duration * 48 )
    }
    else {
        DELAY_CPU_TICKS( duration << ( SystemCoreClock >> 21 ) )
    }
}
