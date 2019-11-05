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
#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>

#define GPIO_FUNC_POS 20
#define OUTPUT ( 0x1 << GPIO_FUNC_POS )
#define INPUT ( 0x2 << GPIO_FUNC_POS )
#define INPUT_PULLUP ( INPUT | ( 0x4 << GPIO_FUNC_POS ) )
#define INPUT_PULLDOWN ( INPUT | ( 0x8 << GPIO_FUNC_POS ) )
#define TRI_STATE ( 0x10 << GPIO_FUNC_POS )
#define GPIO_FUNC_MASK \
    ( OUTPUT | INPUT | INPUT_PULLUP | INPUT_PULLDOWN | TRI_STATE )

#ifdef __cplusplus
extern "C" {
#endif

void    pinMode( uint32_t dwPin, uint32_t dwMode );
void    digitalWrite( uint32_t dwPin, uint32_t dwVal );
uint8_t digitalRead( uint32_t ulPin );

#ifdef __cplusplus
}
#endif

#endif /* GPIO_H_ */
