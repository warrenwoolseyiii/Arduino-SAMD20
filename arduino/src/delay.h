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

#ifndef DELAY_H_
#define DELAY_H_

#include <stdint.h>
#include "variant.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint64_t tix;
    uint32_t rtnMicros;
    uint8_t  inside;
} Micros_Debug_t;

typedef struct
{
    uint64_t mic;
    uint8_t  inside;
} DelayMicros_Debug_t;

void     getMicrosDebugInfo( Micros_Debug_t *mic, DelayMicros_Debug_t *dMic );
uint32_t millis();
void     delay( uint32_t ms );
uint32_t micros();
void     delayMicroseconds( uint32_t us );

#ifdef __cplusplus
}
#endif

#endif /* DELAY_H_ */
