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

#ifndef TIMERCOUNTER_H_
#define TIMERCOUNTER_H_

#include <stdint.h>
#include "sam.h"

typedef enum { tc_mode_8_bit, tc_mode_16_bit, tc_mode_32_bit } TCMode_t;

class TimerCounter
{
  public:
    TimerCounter( Tc *timerCounter );
    void     registerISR( void ( *isr )() );
    void     deregisterISR();
    void     beginPWM( uint32_t frequency, uint8_t dutyCycle );
    void     begin( uint32_t frequency, bool output = false,
                    TCMode_t mode = tc_mode_16_bit, bool useInterrupts = false );
    void     reset();
    void     end();
    void     resume();
    void     pause();
    void     IrqHandler();
    uint32_t getCount();
    void     setCount( uint32_t count );
    bool     isActive()
    {
        return _isActive;
    }
    void setPWMDutyCycle( uint8_t dutyCycle );

  private:
    int8_t   _tcNum;
    bool     _isPaused;
    bool     _isActive;
    uint32_t _maxFreq;
    uint32_t _ccVal;
    uint32_t _ctrlA;
    Tc *     _timerCounter;
    TCMode_t _mode;
    uint32_t _APBCMask;
    uint32_t _clkID;
    uint32_t _irqn;
    void ( *isrPtr )();
    void setDividerAndCC( uint32_t freq, uint32_t maxCC );
    void waitRegSync();
};

#endif /* TIMERCOUNTER_H_ */