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
#include "sam.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _EPortType
{
    NOT_A_PORT = -1,
    PORTA = 0,
    PORTB = 1,
    PORTC = 2,
} EPortType;

typedef struct
{
    EPortType port;
    uint32_t  pin;
    int8_t    extInt;
    int8_t    analog;
    int8_t    spi;
    int8_t    i2c;
    int8_t    uart;
    int8_t    timer;
    int8_t    clkOut;
} ArduinoGPIO_t;

extern const ArduinoGPIO_t gArduinoPins[];

#ifdef __cplusplus
} // extern "C"
#endif
