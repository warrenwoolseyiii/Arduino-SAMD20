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

typedef struct
{
    uint8_t  hadToInit, usedUpdatedCount, insideGet, inside;
    uint64_t tixReturned;
} CPUTix_Debug_t;

typedef struct
{
    uint8_t  hadToInit, inside;
    uint64_t start, cnt, tix;
} DelayCPUTix_Debug_t;

void     initSysTick();
void     disableSysTick();
uint64_t getCPUTicks();
void     delayCPUTicks( uint64_t tix );
void     getCPUTixDebugInfo( CPUTix_Debug_t *tix, DelayCPUTix_Debug_t *dTix,
                             uint8_t *init );

#ifdef __cplusplus
}
#endif

#endif /* SYSTICK_H_ */
