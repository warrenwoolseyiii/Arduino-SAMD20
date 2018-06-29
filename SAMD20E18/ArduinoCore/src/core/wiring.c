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

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * System Core Clock is at 1MHz (8MHz/8) at Reset.
 * It is switched to 48MHz in the Reset Handler (startup.c)
 */
uint32_t SystemCoreClock=1000000ul ;

#define WAIT_GCLK_SYNC while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )

void enableClockNVIC( uint32_t *periph, uint32_t genClk, uint32_t prio )
{
  uint8_t clockId = 0;
  IRQn_Type IdNvic;
  uint32_t APBMask = 0;

  // Make sure the clock source is valid
  if( genClk < GCLK_CLKCTRL_GEN_GCLK0 || genClk > GCLK_CLKCTRL_GEN_GCLK7 )
    return;

  if( periph == SERCOM0 ) {
    clockId = GCM_SERCOM0_CORE;
    IdNvic = SERCOM0_IRQn;
    APBMask = PM_APBCMASK_SERCOM0;
  }
  else if( periph == SERCOM1 ) {
    clockId = GCM_SERCOM1_CORE;
    IdNvic = SERCOM1_IRQn;
    APBMask = PM_APBCMASK_SERCOM1;
  }
  else if( periph == SERCOM2 ) {
    clockId = GCM_SERCOM2_CORE;
    IdNvic = SERCOM2_IRQn;
    APBMask = PM_APBCMASK_SERCOM2;
  }
  else if( periph == SERCOM3 ) {
    clockId = GCM_SERCOM3_CORE;
    IdNvic = SERCOM3_IRQn;
    APBMask = PM_APBCMASK_SERCOM3;
  }
  else {
    return;
  }

  // Interrupt priority
  NVIC_EnableIRQ( IdNvic );
  NVIC_SetPriority( IdNvic, prio );

  // Setting clock
  GCLK->CLKCTRL.reg = ( GCLK_CLKCTRL_ID( clockId ) | genClk | GCLK_CLKCTRL_CLKEN );
  WAIT_GCLK_SYNC;

  PM->APBCMASK.reg |= APBMask;
}

void disableClockNVIC( uint32_t *periph )
{
  uint8_t clockId = 0;
  IRQn_Type IdNvic;
  uint32_t APBMask = 0;

  if( periph == SERCOM0 ) {
    clockId = GCM_SERCOM0_CORE;
    IdNvic = SERCOM0_IRQn;
    APBMask = PM_APBCMASK_SERCOM0;
  }
  else if( periph == SERCOM1 ) {
    clockId = GCM_SERCOM1_CORE;
    IdNvic = SERCOM1_IRQn;
    APBMask = PM_APBCMASK_SERCOM1;
  }
  else if( periph == SERCOM2 ) {
    clockId = GCM_SERCOM2_CORE;
    IdNvic = SERCOM2_IRQn;
    APBMask = PM_APBCMASK_SERCOM2;
  }
  else if( periph == SERCOM3 ) {
    clockId = GCM_SERCOM3_CORE;
    IdNvic = SERCOM3_IRQn;
    APBMask = PM_APBCMASK_SERCOM3;
  }
  else {
    return;
  }

  // Disable interrupt
  NVIC_DisableIRQ( IdNvic );

  // Disable clock
  GCLK->CLKCTRL.reg = ( GCLK_CLKCTRL_ID( clockId ) );
  WAIT_GCLK_SYNC;

  PM->APBCMASK.reg &= ~APBMask;
}

/*
In order to enable a peripheral that is clocked by a Generic Clock, the following 
parts of the system needs to be configured:
 - A running Clock Source. 
 - A clock from the Generic Clock Generator must be configured to use one of the 
  running Clock Sources, and the Generator must be enabled. 
 - The Generic Clock Multiplexer that provides the Generic Clock signal to the peripheral 
  must be configured to use a running Generic Clock Generator, and the Generic Clock must be enabled. 
 - The user interface of the peripheral needs to be unmasked in the PM. If this is not done 
  the peripheral registers will read all 0’s and any writing attempts to the peripheral will be discarded
 SAMD20E18 Data sheet 13.4 "Enabling a Peripheral 
 */

void enableADC()
{
  PM->APBCMASK.reg |= PM_APBCMASK_ADC;
}

void disableADC()
{
  PM->APBCMASK.reg &= ~PM_APBCMASK_ADC;
}

void enableDAC()
{
  PM->APBCMASK.reg |= PM_APBCMASK_DAC;
}

void disableDAC()
{
  PM->APBCMASK.reg &= ~PM_APBCMASK_DAC;
}

int8_t enableSysTick()
{
  int8_t rtn = 0;
  // Set Systick to 1ms interval, common to all Cortex-M variants
  // set Priority for Systick Interrupt (2nd lowest)
  if ( !SysTick_Config( SystemCoreClock / 1000 ) )
    NVIC_SetPriority (SysTick_IRQn,  (1 << __NVIC_PRIO_BITS) - 2);
  else
    rtn = 1;
  return rtn;
}

void disableSysTick()
{
  SysTick->CTRL &= ~( SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk );
  SCB->ICSR |= ( SCB_ICSR_PENDSTCLR_Msk );
}

/*
 * Arduino Zero board initialization
 *
 * Good to know:
 *   - At reset, ResetHandler did the system clock configuration. Core is running at 48MHz.
 *   - Watchdog is disabled by default, unless someone plays with NVM User page
 *   - During reset, all PORT lines are configured as inputs with input buffers, output buffers and pull disabled.
 */
void init( void )
{
  if( enableSysTick() )
    while( 1 );

  enableADC();
  enableDAC();

  // Clock TC/TCC for Pulse and Analog
#ifndef SAMD20
  PM->APBCMASK.reg |= PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 
  | PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5 ;
#else
  PM->APBCMASK.reg |= PM_APBCMASK_TC0 | PM_APBCMASK_TC1 | PM_APBCMASK_TC2 
  | PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5 ;
#endif /* SAMD20 */

  // Setup all pins (digital and analog) in INPUT mode (default is nothing)
  for (uint32_t ul = 0 ; ul < NUM_DIGITAL_PINS ; ul++ )
  {
    pinMode( ul, INPUT ) ;
  }

  // Initialize Analog Controller
  // Setting clock
  while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCM_ADC ) | // Generic Clock ADC
                      GCLK_CLKCTRL_GEN_GCLK0     | // Generic Clock Generator 0 is source
                      GCLK_CLKCTRL_CLKEN ;

  while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains

  ADC->CTRLB.reg = ADC_CTRLB_PRESCALER_DIV512 |    // Divide Clock by 512.
                   ADC_CTRLB_RESSEL_10BIT;         // 10 bits resolution as default

  ADC->SAMPCTRL.reg = 0x3f;                        // Set max Sampling Time Length

  while( ADC->STATUS.bit.SYNCBUSY == 1 );          // Wait for synchronization of registers between the clock domains

  ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND;   // No Negative input (Internal Ground)

  // Averaging (see datasheet table in AVGCTRL register description)
  ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |    // 1 sample only (no oversampling nor averaging)
                     ADC_AVGCTRL_ADJRES(0x0ul);   // Adjusting result by 0

  //analogReference( AR_DEFAULT ) ; // Analog Reference is AREF pin (3.3v)
  analogReference( AR_INTERNAL1V0 );

  // Initialize DAC
  // Setting clock
  while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY );
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCM_DAC ) | // Generic Clock ADC
                      GCLK_CLKCTRL_GEN_GCLK0     | // Generic Clock Generator 0 is source
                      GCLK_CLKCTRL_CLKEN ;

  while ( DAC->STATUS.bit.SYNCBUSY == 1 ); // Wait for synchronization of registers between the clock domains
  DAC->CTRLB.reg = DAC_CTRLB_REFSEL_AVCC | // Using the 3.3V reference
                   DAC_CTRLB_EOEN ;        // External Output Enable (Vout)
}

#ifdef __cplusplus
}
#endif
