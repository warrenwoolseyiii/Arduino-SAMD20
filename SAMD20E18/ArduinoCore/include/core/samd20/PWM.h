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

#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>
#include "TimerCounter.h"

class PWM
{
  public:
    PWM( TimerCounter *tc );
    void begin( uint32_t frequency, uint8_t dutyCycle );
    void setDutyCycle( uint8_t dutyCycle );
    void pause();
    void resume();
    void end();

  private:
    bool          _isPaused;
    TimerCounter *_timer;
};

#endif /* PWM_H_ */