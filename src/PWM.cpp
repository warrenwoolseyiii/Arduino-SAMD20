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

#include "PWM.h"

#define CHECK_DUTY_CYCLE( x ) ( x > 100 ? 100 : x )

PWM::PWM( TimerCounter *tc )
{
    _isPaused = false;
    _timer = tc;
}

void PWM::begin( uint32_t frequency, uint8_t dutyCycle )
{
    _timer->end();
    _timer->beginPWM( frequency, CHECK_DUTY_CYCLE( dutyCycle ) );
}

void PWM::setDutyCycle( uint8_t dutyCycle )
{
    _timer->setPWMDutyCycle( CHECK_DUTY_CYCLE( dutyCycle ) );
}

void PWM::pause()
{
    _timer->pause();
}

void PWM::resume()
{
    _timer->resume();
}

void PWM::end()
{
    _timer->end();
}