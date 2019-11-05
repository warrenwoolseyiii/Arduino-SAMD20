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

#include "SysTick.h"

volatile uint64_t _sysTickUnderFlows;
volatile uint8_t  _useUpdatedCnt;
uint8_t           _insideGet;

void SysTick_IRQHandler()
{
    if( ( _sysTickUnderFlows++ ) > 0xFFFFFFFFFF ) _sysTickUnderFlows = 0;

    // If we are inside the GET_CPU_TICKS macro then we need to update the
    // externally available _useUpdatedCnt field and alert the GET_CPU_TICKS
    // macro that an overflow has occurred. When we re-enter GET_CPU_TICKS the
    // updated under flow counter and new register value should be used.
    if( _insideGet ) _useUpdatedCnt = 1;
}

void initSysTick()
{
    SysTick->CTRL = 0;
    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
    SysTick_Config( SYS_TICK_UNDERFLOW ); // Max value is 2^24 ticks
    _sysTickUnderFlows = 0;
    _useUpdatedCnt = 0;
    _insideGet = 0;
}

void disableSysTick()
{
    SysTick->CTRL = 0;
    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
}

// WARNING: In the deepest sleep mode (STANDBY) the CPU is disabled and the
// SysTick counter is disabled. Upon waking the CPU ticks counter will be
// completely restarted. If you wish to use a timing mechanism that persists
// through STANDBY sleep mode use the stepsRTC() functionality.
uint64_t getCPUTicks()
{
    uint64_t tix;
    GET_CPU_TICKS( tix )
    return tix;
}

void delayCPUTicks( uint64_t tix )
{
    DELAY_CPU_TICKS( tix )
}
