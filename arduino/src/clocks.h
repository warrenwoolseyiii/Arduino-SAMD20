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
#ifndef CLOCKS_H_
#define CLOCKS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void   resetGCLK();
int8_t initClkGenerator( uint32_t clkSrc, uint32_t id, uint32_t div,
                         uint8_t runInStdBy, uint8_t outPutToPin );
void   disableClkGenerator( uint32_t id );
int8_t initGenericClk( uint32_t genClk, uint32_t id );
void   disableGenericClk( uint32_t id );
void   enableAPBAClk( uint32_t item, uint8_t enable );
void   enableAPBBClk( uint32_t item, uint8_t enable );
void   enableAPBCClk( uint32_t item, uint8_t enable );
void   initXOSC32();
void   initOSC23K();
void   initDFLL48( uint32_t sourceFreq );
void   disableDFLL48();
int8_t initOSC8M( uint32_t divBits );
void   disableOSC8M();

#ifdef __cplusplus
}
#endif

#endif /* CLOCKS_H_ */