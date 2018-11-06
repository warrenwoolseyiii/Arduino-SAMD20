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

#define SYS_TICK_UNDERFLOW 0xFFFFFF
#define SYS_TICK_MAXUNDERFLOWS 0xFFFFFFFFFF

volatile uint64_t _sysTickUnderFlows = 0;

void SysTick_IRQHandler()
{
    uint32_t cnt = SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk;
    if( ( _sysTickUnderFlows++ ) > 0xFFFFFFFFFF ) _sysTickUnderFlows = 0;
}

void initSysTick()
{
    SysTick->CTRL = 0;
    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
    SysTick_Config( SYS_TICK_UNDERFLOW ); // Max value is 2^24 ticks
    _sysTickUnderFlows = 0;
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
    // We cannot have the counter register underflow while reading the overflow
    // value, so if we are very close to an underflow, wait for the interrupt
    // request to trigger then perform the calculation.
    if( SysTick->VAL < 64 ) {
        uint32_t prim = __get_PRIMASK();
        __disable_irq();
        while( !( SCB->ICSR & SCB_ICSR_PENDSTSET_Msk ) )
            ;
        if( !prim ) __enable_irq();
    }
    return ( ( _sysTickUnderFlows << 24 ) |
             ( SYS_TICK_UNDERFLOW - SysTick->VAL ) );
}

void delayCPUTicks( uint64_t tix )
{
    uint64_t start = getCPUTicks();
    while( ( getCPUTicks() - start ) < tix )
        ;
}