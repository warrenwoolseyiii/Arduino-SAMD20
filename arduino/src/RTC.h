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
#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>

#define RTC_STEPS_PER_SEC 0x8000
#define RTC_STEPS_OVERFLOW 0x7FFF
#define RTC_OVF_MSB 0x4000

// Rough operations a faster but less accurate
#define RTC_ROUGH_STEPS_TO_MILLIS( x ) ( x >> 5 )
#define RTC_ROUGH_MILLIS_TO_STEPS( x ) ( x << 5 )

// Exact operations are slower but more accurate
#define RTC_EXACT_STEPS_TO_MILLIS( x ) ( ( x * 1000 ) >> 15 )
#define RTC_EXACT_MILLIS_TO_STEPS( x ) ( ( x << 15 ) / 1000 )

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t  overFlowCalled, inside;
    uint32_t rtcFlags, countReg;
    uint64_t rtnSteps;
} RTCSteps_Debug_t;

typedef struct
{
    uint64_t steps, start, cnt;
    int64_t  internalDelay;
    uint8_t  sleepCounter, inside;
} DelayRTCSteps_Debug_t;

typedef struct
{
    uint32_t irqFlags;
    uint8_t  userISRCalled, maxOVF, inside;
} RTCIRQ_Debug_t;

void     initRTC();
void     disableRTC();
uint64_t stepsRTC();
uint64_t secondsRTC();
void     delayRTCSteps( uint64_t steps );
void     delayRTCStepsIdle( uint64_t steps, void ( *idleFunc )() );
void     registerOverflowISR( void ( *ISRFunc )() );
void     registerIdleTaskHasWork( int ( *Func )() );
void getRTCDebugInfo( RTCSteps_Debug_t *steps, DelayRTCSteps_Debug_t *dSteps,
                      RTCIRQ_Debug_t *irqSteps );

#ifdef __cplusplus
}
#endif

#endif /* RTC_H_ */
