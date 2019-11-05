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

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "sam.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_TICK_MSB 0x800000
#define SYS_TICK_UNDERFLOW 0xFFFFFF
#define SYS_TICK_MAXUNDERFLOWS 0xFFFFFFFFFF

extern volatile uint64_t _sysTickUnderFlows;
extern volatile uint8_t  _useUpdatedCnt;
extern uint8_t           _insideGet;

// Macros can be made available globally if the fastest speed is required.
// Internal functions use these same macros so execution is no different.

// The _insideGet field is used to ensure the properly updated underflow and
// tick counter value is used when the underflow ISR interrupts us in the middle
// of the GET_CPU_TICKS macro.
#define GET_CPU_TICKS( x )                                 \
    {                                                      \
        _insideGet = 1;                                    \
        x = ( ( _sysTickUnderFlows << 24 ) |               \
              ( SYS_TICK_UNDERFLOW - SysTick->VAL ) );     \
                                                           \
        if( _useUpdatedCnt ) {                             \
            x = ( ( _sysTickUnderFlows << 24 ) |           \
                  ( SYS_TICK_UNDERFLOW - SysTick->VAL ) ); \
            _useUpdatedCnt = 0;                            \
        }                                                  \
                                                           \
        _insideGet = 0;                                    \
    }

#define DELAY_CPU_TICKS( tix )            \
    {                                     \
        uint64_t start, cnt;              \
        GET_CPU_TICKS( start )            \
        do {                              \
            GET_CPU_TICKS( cnt )          \
        } while( ( cnt - start ) < tix ); \
    }

void     initSysTick();
void     disableSysTick();
uint64_t getCPUTicks();
void     delayCPUTicks( uint64_t tix );

#ifdef __cplusplus
}
#endif

#endif /* SYSTICK_H_ */
