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
#include "variant.h"
#include "clocks.h"

#include <stdio.h>

/**
 * \brief SystemInit() configures the needed clocks and according Flash Read Wait States.
 * At reset:
 * - OSC8M clock source is enabled with a divider by 8 (1MHz).
 * - Generic Clock Generator 0 (GCLKMAIN) is using OSC8M as source.
 * We need to:
 * 1) Enable XOSC32K clock (External on-board 32.768Hz oscillator), will be used as DFLL48M reference.
 * 2) Put XOSC32K as source of Generic Clock Generator 1
 * 3) Put Generic Clock Generator 1 as source for Generic Clock Multiplexer 0 (DFLL48M reference)
 * 4) Enable DFLL48M clock
 * 5) Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
 * 6) Modify PRESCaler value of OSCM to have 8MHz
 * 7) Put OSC8M as source for Generic Clock Generator 3
 */
// Constants for Clock generators
#define GENERIC_CLOCK_GENERATOR_MAIN      (0u)
#define GENERIC_CLOCK_GENERATOR_XOSC32K   (1u)
#define GENERIC_CLOCK_GENERATOR_OSC32K    (1u)
#define GENERIC_CLOCK_GENERATOR_OSCULP32K (2u) /* Initialized at reset for WDT */
#define GENERIC_CLOCK_GENERATOR_OSC8M     (3u)
// Constants for Clock multiplexers
#define GENERIC_CLOCK_MULTIPLEXER_DFLL48M (0u)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define GCLK_WAIT_SYNC          while( GCLK->STATUS.bit.SYNCBUSY )
#define SYSCTRL_DFLL_WAIT_SYNC  while ( !SYSCTRL->PCLKSR.bit.DFLLRDY )

inline void loadADCFactoryCal()
{
  // ADC Bias Calibration
  uint32_t bias = (*((uint32_t *) ADC_FUSES_BIASCAL_ADDR) & ADC_FUSES_BIASCAL_Msk) >> ADC_FUSES_BIASCAL_Pos;

  // ADC Linearity bits 4:0
  uint32_t linearity = (*((uint32_t *) ADC_FUSES_LINEARITY_0_ADDR) & ADC_FUSES_LINEARITY_0_Msk) >> ADC_FUSES_LINEARITY_0_Pos;

  // ADC Linearity bits 7:5
  linearity |= ((*((uint32_t *) ADC_FUSES_LINEARITY_1_ADDR) & ADC_FUSES_LINEARITY_1_Msk) >> ADC_FUSES_LINEARITY_1_Pos) << 5;

  ADC->CALIB.reg = ADC_CALIB_BIAS_CAL(bias) | ADC_CALIB_LINEARITY_CAL(linearity);
}

void SystemInit( void )
{
  /* Set 1 Flash Wait State for 48MHz, cf tables 20.9 and 35.27 in SAMD21 Datasheet */
  NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val ;

  /* Turn on the digital interface clock */
  PM->APBAMASK.reg |= PM_APBAMASK_GCLK ;


#if defined(CRYSTALLESS)

  /* ----------------------------------------------------------------------------------------------
   * 1) Enable OSC32K clock (Internal 32.768Hz oscillator)
   */
  uint32_t calib =
#ifndef SAMD20
        (*((uint32_t *) FUSES_OSC32K_CAL_ADDR) & FUSES_OSC32K_CAL_Msk) >> FUSES_OSC32K_CAL_Pos;
#else
#define FUSES_OSC32K_CAL_ADDR (uint32_t)(0x806020 + 38) // SAMD20 datasheet section 9.5, table 9-4
        (*((uint32_t *) FUSES_OSC32K_CAL_ADDR) & 0x00FFFFFF);
#endif /* SAMD20 */

  SYSCTRL->OSC32K.reg = SYSCTRL_OSC32K_CALIB(calib) |
                        SYSCTRL_OSC32K_STARTUP( 0x6u ) | // cf table 15.10 of product datasheet in chapter 15.8.6
                        SYSCTRL_OSC32K_EN32K |
                        SYSCTRL_OSC32K_ENABLE;

  while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_OSC32KRDY) == 0 ); // Wait for oscillator stabilization

#else // has crystal

  /* ----------------------------------------------------------------------------------------------
   * 1) Enable XOSC32K clock (External on-board 32.768Hz oscillator)
   */
  SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_STARTUP( 0x6u ) | /* cf table 15.10 of product datasheet in chapter 15.8.6 */
                         SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K ;
  SYSCTRL->XOSC32K.bit.ENABLE = 1 ; /* separate call, as described in chapter 15.6.3 */

  while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY) == 0 )
  {
    /* Wait for oscillator stabilization */
  }

#endif

  /* Software reset the module to ensure it is re-initialized correctly */
  /* Note: Due to synchronization, there is a delay from writing CTRL.SWRST until the reset is complete.
   * CTRL.SWRST and STATUS.SYNCBUSY will both be cleared when the reset is complete, as described in chapter 13.8.1
   */
  GCLK->CTRL.reg = GCLK_CTRL_SWRST ;

  while ( (GCLK->CTRL.reg & GCLK_CTRL_SWRST) && (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) )
  {
    /* Wait for reset to complete */
  }

  /* ----------------------------------------------------------------------------------------------
   * 2) Put XOSC32K as source of Generic Clock Generator 1
   */
  GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_XOSC32K ) ; // Generic Clock Generator 1

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
  {
    /* Wait for synchronization */
  }

  /* Write Generic Clock Generator 1 configuration */
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_OSC32K ) | // Generic Clock Generator 1
#if defined(CRYSTALLESS)
                      GCLK_GENCTRL_SRC_OSC32K | // Selected source is Internal 32KHz Oscillator
#else
                      GCLK_GENCTRL_SRC_XOSC32K | // Selected source is External 32KHz Oscillator
#endif
//                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
                      GCLK_GENCTRL_GENEN ;

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
  {
    /* Wait for synchronization */
  }

  /* ----------------------------------------------------------------------------------------------
   * 3) Put Generic Clock Generator 1 as source for Generic Clock Multiplexer 0 (DFLL48M reference)
   */
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GENERIC_CLOCK_MULTIPLEXER_DFLL48M ) | // Generic Clock Multiplexer 0
                      GCLK_CLKCTRL_GEN_GCLK1 | // Generic Clock Generator 1 is source
                      GCLK_CLKCTRL_CLKEN ; // Enables

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
  {
    /* Wait for synchronization */
  }

  /* ----------------------------------------------------------------------------------------------
   * 4) Enable DFLL48M clock
   */

  /* DFLL Configuration in Closed Loop mode, cf product datasheet chapter 15.6.7.1 - Closed-Loop Operation */

  /* Remove the OnDemand mode, Bug http://avr32.icgroup.norway.atmel.com/bugzilla/show_bug.cgi?id=9905 */
  SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;

  while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
  {
    /* Wait for synchronization */
  }

  SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP( 31 ) | // Coarse step is 31, half of the max value
                         SYSCTRL_DFLLMUL_FSTEP( 511 ) | // Fine step is 511, half of the max value
                         SYSCTRL_DFLLMUL_MUL( (VARIANT_MCK + VARIANT_MAINOSC/2) / VARIANT_MAINOSC ) ; // External 32KHz is the reference

  while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
  {
    /* Wait for synchronization */
  }

#if defined(CRYSTALLESS)

  #define NVM_SW_CALIB_DFLL48M_COARSE_VAL 58

  // Turn on DFLL
  uint32_t coarse =( *((uint32_t *)(NVMCTRL_OTP4) + (NVM_SW_CALIB_DFLL48M_COARSE_VAL / 32)) >> (NVM_SW_CALIB_DFLL48M_COARSE_VAL % 32) )
                   & ((1 << 6) - 1);
  if (coarse == 0x3f) {
    coarse = 0x1f;
  }
  // TODO(tannewt): Load this value from memory we've written previously. There
  // isn't a value from the Atmel factory.
  uint32_t fine = 0x1ff;

  SYSCTRL->DFLLVAL.bit.COARSE = coarse;
  SYSCTRL->DFLLVAL.bit.FINE = fine;
  /* Write full configuration to DFLL control register */
  SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP( 0x1f / 4 ) | // Coarse step is 31, half of the max value
                         SYSCTRL_DFLLMUL_FSTEP( 10 ) |
                         SYSCTRL_DFLLMUL_MUL( (48000) ) ;

  SYSCTRL->DFLLCTRL.reg = 0;

  while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
  {
    /* Wait for synchronization */
  }

#ifndef SAMD20
  SYSCTRL->DFLLCTRL.reg =  SYSCTRL_DFLLCTRL_MODE |
                           SYSCTRL_DFLLCTRL_CCDIS |
                           SYSCTRL_DFLLCTRL_USBCRM | /* USB correction */
                           SYSCTRL_DFLLCTRL_BPLCKC;
#endif

  while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
  {
    /* Wait for synchronization */
  }

  /* Enable the DFLL */
  SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE ;

#else   // has crystal

  /* Write full configuration to DFLL control register */
  SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_MODE | /* Enable the closed loop mode */
#ifndef SAMD20
                           SYSCTRL_DFLLCTRL_WAITLOCK |
#endif /* SAMD20 */
                           SYSCTRL_DFLLCTRL_QLDIS ; /* Disable Quick lock */

  while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
  {
    /* Wait for synchronization */
  }

  /* Enable the DFLL */
  SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE ;

  while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0 ||
          (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0 )
  {
    /* Wait for locks flags */
  }

#endif

  while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
  {
    /* Wait for synchronization */
  }

  /* ----------------------------------------------------------------------------------------------
   * 5) Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
   */
  GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_MAIN ) ; // Generic Clock Generator 0

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
  {
    /* Wait for synchronization */
  }

  /* Write Generic Clock Generator 0 configuration */
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_MAIN ) | // Generic Clock Generator 0
                      GCLK_GENCTRL_SRC_DFLL48M | // Selected source is DFLL 48MHz
//                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
                      GCLK_GENCTRL_IDC | // Set 50/50 duty cycle
                      GCLK_GENCTRL_GENEN ;

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
  {
    /* Wait for synchronization */
  }

  /* ----------------------------------------------------------------------------------------------
   * 6) Modify PRESCaler value of OSC8M to have 8MHz
   */
  SYSCTRL->OSC8M.bit.PRESC = 
#ifndef SAMD20
      SYSCTRL_OSC8M_PRESC_0_Val ;  //CMSIS 4.5 changed the prescaler defines
#else
      SYSCTRL_OSC8M_PRESC(0x0);
#endif /* SAMD20 */
  SYSCTRL->OSC8M.bit.ONDEMAND = 0 ;

  /* ----------------------------------------------------------------------------------------------
   * 7) Put OSC8M as source for Generic Clock Generator 3
   */
  GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_OSC8M ) ; // Generic Clock Generator 3

  /* Write Generic Clock Generator 3 configuration */
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_OSC8M ) | // Generic Clock Generator 3
                      GCLK_GENCTRL_SRC_OSC8M | // Selected source is RC OSC 8MHz (already enabled at reset)
//                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
                      GCLK_GENCTRL_GENEN ;

  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
  {
    /* Wait for synchronization */
  }

  /*
   * Now that all system clocks are configured, we can set CPU and APBx BUS clocks.
   * There values are normally the one present after Reset.
   */
  PM->CPUSEL.reg  = PM_CPUSEL_CPUDIV_DIV1 ;
  PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1_Val ;
  PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1_Val ;
  PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1_Val ;

  SystemCoreClock=VARIANT_MCK ;

  /* ----------------------------------------------------------------------------------------------
   * 8) Load ADC factory calibration values
   */

  // ADC Bias Calibration
  uint32_t bias = (*((uint32_t *) ADC_FUSES_BIASCAL_ADDR) & ADC_FUSES_BIASCAL_Msk) >> ADC_FUSES_BIASCAL_Pos;

  // ADC Linearity bits 4:0
  uint32_t linearity = (*((uint32_t *) ADC_FUSES_LINEARITY_0_ADDR) & ADC_FUSES_LINEARITY_0_Msk) >> ADC_FUSES_LINEARITY_0_Pos;

  // ADC Linearity bits 7:5
  linearity |= ((*((uint32_t *) ADC_FUSES_LINEARITY_1_ADDR) & ADC_FUSES_LINEARITY_1_Msk) >> ADC_FUSES_LINEARITY_1_Pos) << 5;

  ADC->CALIB.reg = ADC_CALIB_BIAS_CAL(bias) | ADC_CALIB_LINEARITY_CAL(linearity);

  /*
   * 9) Disable automatic NVM write operations
   */
  NVMCTRL->CTRLB.bit.MANW = 1;
}

/*
 * The low power clock system is all based off the external 32.768 kHz crystal (or the internal). The
 * external crystal drives clock generator 1 and 2. Clock generator 1 is the source for the DFLL 48 MHz 
 * controller for the main CPU which drives clock generator 0.
 */
inline void LowPowerSysInit( void )
{
  // Set 1 Flash Wait State for 48MHz, cf tables 20.9 and 35.27 in SAMD21 Datasheet
  NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val ;
  resetGCLK();

/* ----------------------------------------------------------------------------------------------
 * 0) Disable every clock generator except for 0, then disable all peripherals, these are
 * recommendations laid out in Atmel Application Note AT04188:
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
  uint32_t apbMask = PM_APBAMASK_PAC0 | PM_APBAMASK_WDT | PM_APBAMASK_RTC | PM_APBAMASK_EIC;
  enableAPBAClk( apbMask, 0 );
  apbMask = PM_APBBMASK_PAC1 
#ifndef DEBUG
    | PM_APBBMASK_DSU 
#endif /* DEBUG */
    | PM_APBBMASK_PORT;
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
#if defined(CRYSTALLESS)
  initOSC23K();
#else
  initXOSC32();
#endif

/* ----------------------------------------------------------------------------------------------
 * 2) Put 32K as source of Generic Clock Generator 1 
      -> Freq = 32768 Hz
      -> Runs in stand by
 */
  initClkGenerator( GCLK_GENCTRL_SRC_XOSC32K_Val, GCLK_GENDIV_ID_GCLK1_Val, 
    0, TRUE, FALSE );

///* ----------------------------------------------------------------------------------------------
 //* 3) Put 32K as source of Generic Clock Generator 2 
      //-> Freq = 1024 Hz
      //-> Runs in stand by
 //*/
  //initClkGenerator( GCLK_GENCTRL_SRC_XOSC32K_Val, GCLK_GENDIV_ID_GCLK2_Val,
    //0x04, TRUE, FALSE );
//
///* ----------------------------------------------------------------------------------------------
 //* 4) Put Generic Clock Generator 1 as source for Generic Clock Multiplexer 0 (DFLL48M reference)
 //*/
  //initGenericClk( GCLK_CLKCTRL_GEN_GCLK1_Val, GCLK_CLKCTRL_ID_DFLL48M_Val );
//
///* ----------------------------------------------------------------------------------------------
 //* 5) Enable DFLL48M clock
      //-> Freq = VARIANT_MCK (defined in variant.h)
      //-> Does not run in stand by
 //*/
  //initDFLL48( VARIANT_MAINOSC );
//
///* ----------------------------------------------------------------------------------------------
 //* 6) Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
 //*/
  //initClkGenerator( GCLK_GENCTRL_SRC_DFLL48M_Val, GCLK_GENDIV_ID_GCLK0_Val,
    //0, FALSE, FALSE );
//
  //SystemCoreClock = VARIANT_MCK;

/* ----------------------------------------------------------------------------------------------
 * 7) Enable the OSC8M
 */
  initOSC8M( 0x0 );
  initClkGenerator( GCLK_GENCTRL_SRC_OSC8M_Val, GCLK_GENDIV_ID_GCLK0_Val,
    0, FALSE, FALSE );
  SystemCoreClock = 8000000ul;

///* ----------------------------------------------------------------------------------------------
 //* 8) Load ADC factory calibration values
 //*/
  //loadADCFactoryCal();

/* ----------------------------------------------------------------------------------------------
 * 9) Disable automatic NVM write operations
 */
  NVMCTRL->CTRLB.bit.MANW = 1;
}