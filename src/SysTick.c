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

volatile uint32_t _sysTickUnderFlows = 0;

void SysTick_IRQHandler()
{
	uint32_t cnt = SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk;
	_sysTickUnderFlows++;
}

void initSysTick()
{
	SysTick->CTRL = 0;
	NVIC_DisableIRQ( SysTick_IRQn );
	NVIC_ClearPendingIRQ( SysTick_IRQn );
	SysTick_Config( SYS_TICK_UNDERFLOW );	// Max value is 2^24 ticks
	NVIC_EnableIRQ( SysTick_IRQn );
	_sysTickUnderFlows = 0;
}

uint64_t getCPUTicks()
{	
	// We cannot have the counter register underflow while reading the overflow value, 
	// so if we are very close to an underflow, wait for the interrupt request to trigger
	// then perform the calculation.
	//if( SysTick->VAL < 32 ) {
		//SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
		//while( !( SCB->ICSR & SCB_ICSR_PENDSTSET_Msk ) );
		//SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	//}
	return ( (uint64_t)( _sysTickUnderFlows << 24 ) | ( SYS_TICK_UNDERFLOW - SysTick->VAL ) );
}

void delayCPUTicks( uint64_t tix )
{
	uint64_t start = getCPUTicks();
	while( ( getCPUTicks() - start ) < tix );
}