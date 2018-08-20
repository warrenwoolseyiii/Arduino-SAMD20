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

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * \brief SAMD products have only one reference for ADC
 */
typedef enum _eAnalogReference {
    AR_DEFAULT,
    AR_INTERNAL,
    AR_EXTERNAL,
    AR_INTERNAL1V0,
    AR_INTERNAL1V65,
    AR_INTERNAL2V23
} eAnalogReference;

void     analogReference( eAnalogReference ulMode );
void     analogWrite( uint32_t ulPin, uint32_t ulValue );
uint32_t analogReadVcc();
uint32_t analogRead( uint32_t ulPin );
void     analogReadResolution( int res );
void     analogWriteResolution( int res );
void     analogOutputInit( void );

#ifdef __cplusplus
}
#endif
