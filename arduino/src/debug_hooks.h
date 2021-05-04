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
#ifndef DEBUG_HOOKS_H_
#define DEBUG_HOOKS_H_

#include "sam.h"
#include "delay.h"
#include "WDT.h"
#include "RTC.h"
#include "SysTick.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    Micros_Debug_t        mic;
    DelayMicros_Debug_t   dMic;
    WDT_Debug_t           wdt;
    RTCSteps_Debug_t      rtc;
    DelayRTCSteps_Debug_t dRtc;
    RTCIRQ_Debug_t        rtcIrq;
    CPUTix_Debug_t        tix;
    DelayCPUTix_Debug_t   dTix;
    uint8_t               tixInit;
} Core_Debug_t;

__STATIC_INLINE uint32_t __get_LR( void )
{
    register uint32_t __regLinkRegister __ASM( "lr" );
    return ( __regLinkRegister );
}

void getCoreDebugInfo( Core_Debug_t *dbg );

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_HOOKS_H_ */
