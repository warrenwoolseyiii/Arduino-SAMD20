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

#ifndef TIMERCOUNTER_H_
#define TIMERCOUNTER_H_

class TimerCounter 
{
public:
  TimerCounter( Tc* timerCounter );
  void initTimerCounter( uint32_t frequency, int8_t outputPin = -1 );
  void registerISR( void *isr() );
  void deregisterISR();
  void begin();
  void end();
};

#endif /* TIMERCOUNTER_H_ */