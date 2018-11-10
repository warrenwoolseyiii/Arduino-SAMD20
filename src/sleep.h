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
#ifndef SLEEP_H_
#define SLEEP_H_

#include <stdint.h>

#define PM_SLEEP_STANDBY_Val 0xFF

typedef enum {
    cpu_clk_oscm1 = 0x3,
    cpu_clk_oscm2 = 0x2,
    cpu_clk_oscm4 = 0x1,
    cpu_clk_oscm8 = 0x0,
    cpu_clk_dfll48 = 0x4
} CPUClkSrc_t;

#ifdef __cplusplus
extern "C" {
#endif

void sleepCPU( uint32_t level );
void changeCPUClk( CPUClkSrc_t src );
void disableSleep();
void enableSleep();

#ifdef __cplusplus
}
#endif

#endif /* SLEEP_H_ */