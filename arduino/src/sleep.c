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

#include "sam.h"
#include "sleep.h"
#include "clocks.h"
#include "variant.h"
#include "SysTick.h"
#include "delay.h"

uint8_t  _sleepEn = 1;
uint32_t _sysUpTimeAccum = 0;

volatile uint8_t  _needExitSleep = 0;
volatile uint8_t  _exitLock = 0;
volatile uint32_t _sysExitSleepTime = 0;

void disableSleep()
{
    _sleepEn = 0;
}

void enableSleep()
{
    _sleepEn = 1;
}

void exitSleep()
{
    if( _exitLock ) return;
    _exitLock = 1;

    if( _needExitSleep ) {
        initSysTick();
        _sysExitSleepTime = micros();

        _needExitSleep = 0;
    }

    _exitLock = 0;
}

void sleepCPU( SleepLevel_t level )
{
    _sysUpTimeAccum += micros() - _sysExitSleepTime;

    if( _sleepEn ) {

        _needExitSleep = 1;
        if( level > PM_SLEEP_IDLE_APB_Val ) {
            disableSysTick();
            SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
        }
        else {
            SCB->SCR &= ~( SCB_SCR_SLEEPDEEP_Msk );
            PM->SLEEP.reg = PM_SLEEP_IDLE( level );
        }

        __WFI();

        exitSleep();
    }
}

void changeCPUClk( CPUClkSrc_t src )
{
    disableSysTick();

    if( src < cpu_clk_dfll48 ) {
        initOSC8M( src );
        initClkGenerator( GCLK_GENCTRL_SRC_OSC8M_Val, GCLK_GENDIV_ID_GCLK0_Val,
                          0, 0, 0 );
        SystemCoreClock = 8000000ul / ( 1 << src );
        disableDFLL48();
        disableGenericClk( GCLK_CLKCTRL_ID_DFLL48M_Val );

        // Under 24 MHz at 2.7V & up requires 0 wait states, section 32.11 NVM
        // Characteristics
        NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_SINGLE_Val;
    }
    else {
        // Transitioning to 48 MHz at 2.7V & up requires 1 wait state,
        // section 32.11 NVM Characteristics
        NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val;

        initGenericClk( GCLK_CLKCTRL_GEN_GCLK1_Val,
                        GCLK_CLKCTRL_ID_DFLL48M_Val );
        initDFLL48( VARIANT_MAINOSC );
        initClkGenerator( GCLK_GENCTRL_SRC_DFLL48M_Val,
                          GCLK_GENDIV_ID_GCLK0_Val, 0, 0, 0 );
        SystemCoreClock = VARIANT_MCK;
        disableOSC8M();
    }

    // If we change the clock frequency the SysTick timer will need to be
    // restarted
    initSysTick();
}

uint32_t getSysUpTime()
{
    return _sysUpTimeAccum;
}
