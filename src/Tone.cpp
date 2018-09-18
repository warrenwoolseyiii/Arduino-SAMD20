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

#include "Tone.h"

uint64_t _durationCounts;
uint64_t _counter;
uint8_t  _channel;

void toneISR()
{
    if( ++_counter > _durationCounts ) {
        switch( _channel ) {
            case 0: Timer.end(); break;
            case 1: Timer1.end(); break;
            case 2: Timer2.end(); break;
            case 3: Timer3.end(); break;
        }
    }
}

bool tone( uint8_t channel, uint32_t frequency, uint32_t durationSeconds )
{
    bool rtn = false;
    _durationCounts = frequency * durationSeconds;
    if( _durationCounts > 0 ) {

        _counter = 0;
        _channel = channel;

        switch( channel ) {
            case 0:
                Timer.end();
                Timer.deregisterISR();
                Timer.registerISR( toneISR );
                Timer.begin( frequency, true, tc_mode_16_bit, true );
                rtn = true;
                break;
            case 1:
                Timer1.end();
                Timer1.deregisterISR();
                Timer1.registerISR( toneISR );
                Timer1.begin( frequency, true, tc_mode_16_bit, true );
                rtn = true;
                break;
            case 2:
                Timer2.end();
                Timer2.deregisterISR();
                Timer2.registerISR( toneISR );
                Timer2.begin( frequency, true, tc_mode_16_bit, true );
                rtn = true;
                break;
            case 3:
                Timer3.end();
                Timer3.deregisterISR();
                Timer3.registerISR( toneISR );
                Timer3.begin( frequency, true, tc_mode_16_bit, true );
                rtn = true;
                break;
        }
    }
    return rtn;
}
