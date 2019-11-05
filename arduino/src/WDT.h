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
#ifndef WDT_H_
#define WDT_H_

#include <stdint.h>
#include "sam.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    wdt_8_ms = WDT_CONFIG_PER_8_Val,
    wdt_16_ms = WDT_CONFIG_PER_16_Val,
    wdt_32_ms = WDT_CONFIG_PER_32_Val,
    wdt_64_ms = WDT_CONFIG_PER_64_Val,
    wdt_128_ms = WDT_CONFIG_PER_128_Val,
    wdt_256_ms = WDT_CONFIG_PER_256_Val,
    wdt_512_ms = WDT_CONFIG_PER_512_Val,
    wdt_1_s = WDT_CONFIG_PER_1K_Val,
    wdt_2_s = WDT_CONFIG_PER_2K_Val,
    wdt_4_s = WDT_CONFIG_PER_4K_Val,
    wdt_8_s = WDT_CONFIG_PER_8K_Val,
    wdt_16_s = WDT_CONFIG_PER_16K_Val
} WDTPeriod_t;

void initWDT( WDTPeriod_t wdtPeriod );
void endWDT();
void enableWDT();
void disableWDT();
void clearWDT();
void resetCPU();

#ifdef __cplusplus
}
#endif

#endif /* WDT_H_ */
