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

uint32_t millis()
{
    uint64_t steps = stepsRTC();
    return RTC_EXACT_STEPS_TO_MILLIS( steps );
}

void delay( uint32_t ms )
{
    int64_t start, steps, count;
    steps = RTC_EXACT_MILLIS_TO_STEPS( ms );
    if( ms == 1 ) steps++;
    start = stepsRTC();

    do {
        yield();
        count = stepsRTC();
    } while( ( count - start ) < steps );
}
