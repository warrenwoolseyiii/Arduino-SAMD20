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
#include "WDT.h"
#include "sam.h"
#include "clocks.h"

#define WDT_WAIT_SYNC while( WDT->STATUS.bit.SYNCBUSY )

uint8_t _isInit = 0;

/* Note: The WDT runs off the 32768 Hz clock source divided by 32, giving the
 * WDT a 1024 Hz clock source. */
void initWDT( uint32_t wdtPeriod )
{
    // Set up the clocks and APB
    initClkGenerator( GCLK_GENCTRL_SRC_XOSC32K_Val, GCLK_GENDIV_ID_GCLK2_Val,
                      0x04, 0x1, 0x0 );
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK2_Val, GCLK_CLKCTRL_ID_WDT_Val );
    enableAPBAClk( PM_APBAMASK_WDT, 1 );

    // Reset the control reg
    disableWDT();
    WDT->CTRL.reg = 0;
    WDT_WAIT_SYNC;

    // Ensure all interrupts are cleared
    WDT->INTENCLR.bit.EW = 1;
    NVIC_DisableIRQ( WDT_IRQn );

    // Set the period
    if( wdtPeriod > WDT_CONFIG_PER_16K_Val ) wdtPeriod = WDT_CONFIG_PER_16K_Val;
    WDT->CONFIG.reg = WDT_CONFIG_PER( wdtPeriod );

    enableWDT();
}

void endWDT()
{
    disableWDT();
    enableAPBAClk( PM_APBAMASK_WDT, 0 );
    disableGenericClk( GCLK_CLKCTRL_ID_WDT_Val );
    disableClkGenerator( GCLK_GENDIV_ID_GCLK2_Val );
}

void enableWDT()
{
    WDT->CTRL.bit.ENABLE = 1;
    WDT_WAIT_SYNC;
    _isInit = 1;
}

void disableWDT()
{
    WDT->CTRL.bit.ENABLE = 0;
    WDT_WAIT_SYNC;
    _isInit = 0;
}

void clearWDT()
{
    // Data sheet Section 17.8.8: Writing 0xA5 will reset the WDT counter
    WDT->CLEAR.reg = 0xA5;
    WDT_WAIT_SYNC;
}

/* Warning! Will not return from here. */
void resetCPU()
{
    if( !_isInit ) initWDT( WDT_CONFIG_PER_8_Val );

    // Data sheet Section 17.8.8: Writing any value other than 0xA5 will reset
    // the CPU immediately
    WDT->CLEAR.reg = 0xFF;
    WDT_WAIT_SYNC;
}