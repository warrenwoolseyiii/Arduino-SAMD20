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
#include <Arduino.h>

#if defined( MICRO_TIMER )
TimerCounter _timerMicros( TC0 );
uint32_t _micros = 0;
bool _isPaused = false;


void beginMicroTimer()
{
    _timerMicros.begin( 1, -1, tc_mode_32_bit, false );
}

void resumeMicroTimer()
{
    if( _isPaused ) {
        uint32_t seconds = secondsRTC();
        uint32_t count = RTC_ROUGH_STEPS_TO_MICROS( ( ( seconds << 15 ) - stepsRTC() ) );
        
        _micros = ( seconds * 1000000 ) + count;

        _timerMicros.setCount( count );
        _timerMicros.resume();
        _isPaused = false;
    }
}

void pauseMicroTimer()
{
    if( !_isPaused ) {
        _timerMicros.pause();
        _isPaused = true;
    }
}

#ifdef __cplusplus
extern "C" {
#endif
void resetMicroTimerCount()
{
    _timerMicros.setCount( 0 );
}
#ifdef __cplusplus
}
#endif

#endif /* MICRO_TIMER */

uint32_t micros()
{
#if defined( MICRO_TIMER )
    if( _isPaused )
        resumeMicroTimer();
    else
        _micros = ( secondsRTC() * 1000000 ) + _timerMicros.getCount();

    return _micros;
#else
    return ( RTC_ROUGH_STEPS_TO_MICROS( stepsRTC() ) );
#endif /* MICRO_TIMER */ 
}

void delayMicroseconds( uint32_t us )
{
    uint32_t start = micros();
    do {
        yield();
    } while( micros() - start < us );
}
