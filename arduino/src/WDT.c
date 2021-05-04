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
#include "atomic.h"
#include "debug_hooks.h"
#include "delay.h"

#define WDT_SYNC_BUSY WDT->STATUS.bit.SYNCBUSY
#define WDT_WAIT_SYNC while( WDT_SYNC_BUSY )

uint8_t _isInit = 0;
void ( *userEarlyWarningISR )( uint32_t ) = 0;
void ( *userHardFaultISR )( uint32_t ) = 0;
volatile WDT_Debug_t _wdtDebug = {0, 0, 0};

/* Note: The WDT runs off the 32768 Hz clock source divided by 32, giving the
 * WDT a 1024 Hz clock source. */
void initWDT( WDTPeriod_t wdtPeriod )
{
    // Set up the clocks and APB
    initClkGenerator( GCLK_GENCTRL_SRC_XOSC32K_Val, GCLK_GENDIV_ID_GCLK2_Val,
                      0x04, 0x1, 0x0 );
    initGenericClk( GCLK_CLKCTRL_GEN_GCLK2_Val, GCLK_CLKCTRL_ID_WDT_Val );
    enableAPBAClk( PM_APBAMASK_WDT, 1 );

    // Reset the control reg
    disableWDT();
    ATOMIC_OPERATION( {
        if( WDT_SYNC_BUSY ) WDT_WAIT_SYNC;
        WDT->CTRL.reg = 0;
    } )

    // Ensure all interrupts are cleared
    WDT->INTENCLR.bit.EW = 1;
    NVIC_DisableIRQ( WDT_IRQn );

    // Set the period
    if( wdtPeriod > WDT_CONFIG_PER_16K_Val ) wdtPeriod = WDT_CONFIG_PER_16K_Val;
    WDT->CONFIG.reg = WDT_CONFIG_PER( wdtPeriod );

    if( wdtPeriod > WDT_CONFIG_PER_8_Val ) {
        WDT->EWCTRL.reg = WDT_EWCTRL_EWOFFSET( ( wdtPeriod - 1 ) );
        WDT->INTENSET.bit.EW = 1;

        NVIC_EnableIRQ( WDT_IRQn );
    }

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
    ATOMIC_OPERATION( {
        if( WDT_SYNC_BUSY ) WDT_WAIT_SYNC;
        WDT->CTRL.bit.ENABLE = 1;
    } )

    _isInit = 1;
}

void disableWDT()
{
    ATOMIC_OPERATION( {
        if( WDT_SYNC_BUSY ) WDT_WAIT_SYNC;
        WDT->CTRL.bit.ENABLE = 0;
    } )

    _isInit = 0;
}

uint8_t clearWDT()
{
    _wdtDebug.ok = 0;

    // Data sheet Section 17.8.8: Writing 0xA5 will reset the WDT counter
    ATOMIC_OPERATION( {
        if( !WDT->STATUS.bit.SYNCBUSY ) {
            WDT->CLEAR.reg = 0xA5;
            _wdtDebug.ok = 1;
        }
    } )

    _wdtDebug.caller = __get_LR();
    _wdtDebug.sysTime = millis();

    return _wdtDebug.ok;
}

// Warning! Will not return from here.
void resetCPU()
{
    if( !_isInit ) initWDT( WDT_CONFIG_PER_8_Val );

    // Data sheet Section 17.8.8: Writing any value other than 0xA5 will
    // reset the CPU immediately
    ATOMIC_OPERATION( {
        if( WDT_SYNC_BUSY ) WDT_WAIT_SYNC;
        WDT->CLEAR.reg = 0xFF;
    } )
}

void registerEarlyWarningISR( void ( *ISRFunc )( uint32_t ) )
{
    userEarlyWarningISR = ISRFunc;
}

void registerHardFaultISR( void ( *ISRFunc )( uint32_t ) )
{
    userHardFaultISR = ISRFunc;
}

void WDT_IRQHandler( uint32_t rtnAddr )
{
    if( userEarlyWarningISR != 0 ) userEarlyWarningISR( rtnAddr );
}

void HardFault_IRQHandler( uint32_t rtnAddr )
{
    if( userHardFaultISR != 0 ) userHardFaultISR( rtnAddr );
}

void getWDTDebugInfo( WDT_Debug_t *wdt )
{
    wdt->caller = _wdtDebug.caller;
    wdt->ok = _wdtDebug.ok;
    wdt->sysTime = _wdtDebug.sysTime;
}
