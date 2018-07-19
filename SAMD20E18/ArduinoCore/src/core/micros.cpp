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

TimerCounter _timerMicros( TC0 );
uint32_t _micros = 0;
bool _isPaused = false;

void resume()
{
    if( _isPaused ) {
        _micros = millis() * 1000;
        
        uint32_t count = secondsRTC() * 1000;
        if( _micros < count )
            count = 0;
        else
            count = _micros - count;

        _timerMicros.setCount( count );
        _timerMicros.resume();
        _isPaused = false;
    }
}

uint32_t micros()
{
    if( _isPaused )
        resume();
    else
        _micros += _timerMicros.getCount();

    return _micros;
}
