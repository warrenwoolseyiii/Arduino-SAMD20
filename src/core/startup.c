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

#include "sam.h"
#include "clocks.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define GCLK_WAIT_SYNC while( GCLK->STATUS.bit.SYNCBUSY )
#define SYSCTRL_DFLL_WAIT_SYNC while( !SYSCTRL->PCLKSR.bit.DFLLRDY )

inline void loadADCFactoryCal()
{
    // ADC Bias Calibration
    uint32_t bias =
        ( *( (uint32_t *)ADC_FUSES_BIASCAL_ADDR ) & ADC_FUSES_BIASCAL_Msk ) >>
        ADC_FUSES_BIASCAL_Pos;

    // ADC Linearity bits 4:0
    uint32_t linearity = ( *( (uint32_t *)ADC_FUSES_LINEARITY_0_ADDR ) &
                           ADC_FUSES_LINEARITY_0_Msk ) >>
                         ADC_FUSES_LINEARITY_0_Pos;

    // ADC Linearity bits 7:5
    linearity |= ( ( *( (uint32_t *)ADC_FUSES_LINEARITY_1_ADDR ) &
                     ADC_FUSES_LINEARITY_1_Msk ) >>
                   ADC_FUSES_LINEARITY_1_Pos )
                 << 5;

    ADC->CALIB.reg =
        ADC_CALIB_BIAS_CAL( bias ) | ADC_CALIB_LINEARITY_CAL( linearity );
}

/* The low power system initializes only the peripherals that get used during
 * application. Each module (RTC, WDT, GPIO etc..) automatically enables the module
 * when called. The low power system initializes the 32 kHz crystal oscillator to drive
 * Generic Clock 1, which provides the RTC with a clock source. Generic Clock 0 ( GCLK
 * Main ) is driven by the OSCM8 at 8 MHz. The user can change the main clock frequency
 * by calling changeCPUClk() in the application, which will allow them to run the DFLL
 * at 48 MHz or the OSCM8 at a lower speed.
 */
void LowPowerSysInit( )
{
    NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val;
    resetGCLK();

    /* ----------------------------------------------------------------------------------------------
     * 0) Disable every clock generator except for 0, then disable all
     * peripherals, these are recommendations laid out in Atmel Application Note
     * AT04188:
     * http://ww1.microchip.com/downloads/en/AppNotes/Atmel-42248-SAM-D20-Power-Measurements_ApplicationNote_AT04188.pdf
     */
    // Disable all clock generators except 0
    disableClkGenerator( GCLK_GENDIV_ID_GCLK1_Val );
    disableClkGenerator( GCLK_GENDIV_ID_GCLK2_Val );
    disableClkGenerator( GCLK_GENDIV_ID_GCLK3_Val );
    disableClkGenerator( GCLK_GENDIV_ID_GCLK4_Val );
    disableClkGenerator( GCLK_GENDIV_ID_GCLK5_Val );
    disableClkGenerator( GCLK_GENDIV_ID_GCLK6_Val );
    disableClkGenerator( GCLK_GENDIV_ID_GCLK7_Val );

    // Kill all APB clocks
    uint32_t apbMask =
        PM_APBAMASK_PAC0 | PM_APBAMASK_WDT | PM_APBAMASK_RTC | PM_APBAMASK_EIC;
    enableAPBAClk( apbMask, 0 );
    apbMask = PM_APBBMASK_PAC1 | PM_APBBMASK_DSU | PM_APBBMASK_PORT;
    enableAPBBClk( apbMask, 0 );
    enableAPBCClk( 0xFFFFFFFF, 0 );

    // Hook all disabled peripherals to unused clock generators
    disableGenericClk( GCLK_CLKCTRL_ID_DFLL48M_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_WDT_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_RTC_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_EIC_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_EVSYS_CHANNEL_0_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_EVSYS_CHANNEL_1_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_EVSYS_CHANNEL_2_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_EVSYS_CHANNEL_3_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_EVSYS_CHANNEL_4_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_EVSYS_CHANNEL_5_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_EVSYS_CHANNEL_6_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_EVSYS_CHANNEL_7_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_SERCOMX_SLOW_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_SERCOM0_CORE_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_SERCOM1_CORE_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_SERCOM2_CORE_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_SERCOM3_CORE_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_SERCOM4_CORE_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_SERCOM5_CORE_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_TC0_TC1_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_TC2_TC3_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_TC4_TC5_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_TC6_TC7_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_ADC_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_AC_DIG_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_AC_ANA_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_DAC_Val );
    disableGenericClk( GCLK_CLKCTRL_ID_PTC_Val );
    /* ----------------------------------------------------------------------------------------------
     * 1) Initialize the 32768 Hz oscillator (either, internal or external)
     */
#if defined( CRYSTALLESS )
    initOSC23K();
#else
    initXOSC32();
#endif

    /* ----------------------------------------------------------------------------------------------
     * 2) Put 32K as source of Generic Clock Generator 1
          -> Freq = 32768 Hz
          -> Runs in stand by
     */
    initClkGenerator( GCLK_GENCTRL_SRC_XOSC32K_Val, GCLK_GENDIV_ID_GCLK1_Val, 0,
                      TRUE, FALSE );

    /* ----------------------------------------------------------------------------------------------
     * 3) Enable the OSC8M
     */
    initOSC8M( 0x0 );
    initClkGenerator( GCLK_GENCTRL_SRC_OSC8M_Val, GCLK_GENDIV_ID_GCLK0_Val, 0,
                      FALSE, FALSE );
    SystemCoreClock = 8000000ul;

    /* ----------------------------------------------------------------------------------------------
     * 4) Load ADC factory calibration values
     */
    loadADCFactoryCal();

    /* ----------------------------------------------------------------------------------------------
     * 5) Disable automatic NVM write operations
     */
    NVMCTRL->CTRLB.bit.MANW = 1;
}