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
#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t millis()
{
    return RTC_EXACT_STEPS_TO_MILLIS( stepsRTC() );
}

uint32_t micros()
{
    return RTC_ROUGH_STEPS_TO_MICROS( stepsRTC() );
}

void delay( uint32_t ms )
{
    uint32_t steps = RTC_EXACT_MILLIS_TO_STEPS( ms );
    if( ms == 1 ) steps++;
    uint32_t start = stepsRTC();

    do {
        yield();
    } while( stepsRTC() - start < steps );
}

// WARNING: Due to the frequency the RTC is running at (32768 Hz) the
// smallest increment of time between RTC steps is 30.5 uS.
void delayMicroseconds( uint32_t us )
{
    uint32_t steps = RTC_ROUGH_MICROS_TO_STEPS( us );
    if( steps == 0 ) steps = 32;
    uint32_t start = stepsRTC();

    do {
        yield();
    } while( stepsRTC() - start < steps ); 
}

#ifdef __cplusplus
}
#endif
