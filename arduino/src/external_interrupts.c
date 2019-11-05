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

#include "external_interrupts.h"
#include "Arduino.h"
#include "sam.h"
#include <string.h>

#define EIC_WAIT_SYNC while( EIC->STATUS.bit.SYNCBUSY )

// Callback pointers for each of the external interrupts, plus an extra for the
// non-maskable interrupt
static void ( *ISRcallback[NUM_EXT_INTS + 1] )();

uint8_t _enabled = 0;
uint8_t _lowPowerModeActive = 0;

static void __initialize( uint8_t lowPower )
{
    uint32_t clkSrc = GCLK_CLKCTRL_GEN_GCLK0_Val;

    memset( ISRcallback, 0, sizeof( ISRcallback ) );

    NVIC_DisableIRQ( EIC_IRQn );
    NVIC_ClearPendingIRQ( EIC_IRQn );

    // Enable GCLK for IEC (External Interrupt Controller)
    if( lowPower ) clkSrc = GCLK_CLKCTRL_GEN_GCLK1_Val;
    enableAPBAClk( PM_APBAMASK_EIC, 1 );
    initGenericClk( clkSrc, GCLK_CLKCTRL_ID_EIC_Val );

    NVIC_SetPriority( EIC_IRQn, 0 );
    NVIC_EnableIRQ( EIC_IRQn );

    // Do a software reset on EIC
    EIC->CTRL.bit.SWRST = 1;
    while( ( EIC->CTRL.bit.SWRST ) && ( EIC->STATUS.bit.SYNCBUSY ) )
        ;

    // Enable EIC
    EIC->CTRL.bit.ENABLE = 1;
    EIC_WAIT_SYNC;

    _enabled = 1;
    _lowPowerModeActive = 0;
}

void disableExternalInterrupts()
{
    if( _enabled ) {
        // Do a software reset on EIC
        EIC->CTRL.bit.SWRST = 1;
        while( ( EIC->CTRL.bit.SWRST ) && ( EIC->STATUS.bit.SYNCBUSY ) )
            ;
        _enabled = 0;
    }

    NVIC_DisableIRQ( EIC_IRQn );
    NVIC_ClearPendingIRQ( EIC_IRQn );
    disableGenericClk( GCLK_CLKCTRL_ID_EIC_Val );
    enableAPBAClk( PM_APBAMASK_EIC, 0 );
}

// Selects the clock source for the EIC, low power mode enabled == 32kHz clock
// low power mode disabled == SystemCoreClock
void interruptlowPowerMode( uint8_t en )
{
    if( _enabled ) {
        if( en != _lowPowerModeActive ) {
            // Disable
            EIC->CTRL.bit.ENABLE = 0;
            EIC_WAIT_SYNC;
            NVIC_DisableIRQ( EIC_IRQn );

            // Switch clock sources
            if( en ) {
                enableAPBAClk( PM_APBAMASK_EIC, 1 );
                initGenericClk( GCLK_CLKCTRL_GEN_GCLK1_Val,
                                GCLK_CLKCTRL_ID_EIC_Val );
            }
            else {
                enableAPBAClk( PM_APBAMASK_EIC, 1 );
                initGenericClk( GCLK_CLKCTRL_GEN_GCLK0_Val,
                                GCLK_CLKCTRL_ID_EIC_Val );
            }

            // Enable and reset IRQs
            NVIC_SetPriority( EIC_IRQn, 0 );
            NVIC_EnableIRQ( EIC_IRQn );
            EIC->CTRL.bit.ENABLE = 1;
            EIC_WAIT_SYNC;
        }
    }
    else {
        if( en )
            __initialize( 1 );
        else
            __initialize( 0 );
    }

    _lowPowerModeActive = en;
}

// Sets the pin up for external interrupt mode, registers the callback function
// with that interrupt vector if the callback is not null. Will overwrite
// previous callback function if there was one.
void attachInterrupt( uint32_t pin, void ( *callback )(),
                      uint32_t interruptMode )
{
    uint32_t shifter = gArduinoPins[pin].pin;
    uint32_t EICBit = 0;
    uint8_t  isNMI = 0;

    // No external interrupt on this pin
    if( gArduinoPins[pin].extInt == -1 ) return;

    // Corresponding EIC interrupt number by SAM PORTA pins, mapped directly
    // from the data sheet
    if( shifter < 16 ) {
        if( shifter == 8 ) isNMI = 1;
    }
    else if( shifter >= 16 && shifter < 24 )
        shifter -= 16;
    else if( shifter >= 24 && shifter < 28 )
        shifter -= 12;
    else if( shifter >= 28 && shifter < 32 )
        shifter -= 20;
    else
        return;

    if( !isNMI ) {
        EICBit = 1 << shifter;

        if( !_enabled ) __initialize( 0 );

        // Enable wakeup capability on pin in case being used during sleep
        EIC->WAKEUP.reg |= EICBit;

        // Assign pin to EIC
        pinMode( pin, gArduinoPins[pin].extInt );

        // Ensure that the callback is not null
        if( callback ) {
            ISRcallback[shifter] = callback;

            // Figure out which of the two configuration registers we must use
            // (bottom configuration for bottom 8 external interrupts, top
            // configuration for top 8 external interrupts)
            uint8_t config = ( shifter > 7 ? 1 : 0 );

            // Get the bit mask offset for the bits in the configuration
            // register
            uint32_t pos = shifter * 4;
            if( pos > 28 ) pos /= 2;

            // Configure the interrupt mode
            EIC->CONFIG[config].reg &= ~( EIC_CONFIG_SENSE0_Msk << pos );
            switch( interruptMode ) {
                case LOW:
                    EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_LOW_Val << pos;
                    break;
                case HIGH:
                    EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_HIGH_Val
                                               << pos;
                    break;
                case CHANGE:
                    EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_BOTH_Val
                                               << pos;
                    break;
                case FALLING:
                    EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_FALL_Val
                                               << pos;
                    break;
                case RISING:
                    EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_RISE_Val
                                               << pos;
                    break;
            }
        }

        // Enable the interrupt
        EIC->INTENSET.reg |= EICBit;
    }
    else {
        if( callback ) {
            ISRcallback[NUM_EXT_INTS] = callback;
            EIC->NMICTRL.reg = 0;
            switch( interruptMode ) {
                case LOW: EIC->NMICTRL.reg |= EIC_NMICTRL_NMISENSE_LOW; break;
                case HIGH: EIC->NMICTRL.reg |= EIC_NMICTRL_NMISENSE_HIGH; break;
                case CHANGE:
                    EIC->NMICTRL.reg |= EIC_NMICTRL_NMISENSE_BOTH;
                    break;
                case FALLING:
                    EIC->NMICTRL.reg |= EIC_NMICTRL_NMISENSE_FALL;
                    break;
                case RISING:
                    EIC->NMICTRL.reg |= EIC_NMICTRL_NMISENSE_RISE;
                    break;
            }
        }
    }
}

// Disables the selected pin from external interrupt control and removes
// the callback associated with that pin.
void detachInterrupt( uint32_t pin )
{
    uint32_t shifter = gArduinoPins[pin].pin;
    uint32_t EICBit;
    uint8_t  isNMI = 0;

    // No external interrupt on this pin
    if( gArduinoPins[pin].extInt == -1 ) return;

    // Corresponding EIC interrupt number by SAM PORTA pins, mapped directly
    // from the data sheet
    if( shifter < 16 ) {
        if( shifter == 8 ) isNMI = 1;
    }
    else if( shifter >= 16 && shifter < 24 )
        shifter -= 16;
    else if( shifter >= 24 && shifter < 28 )
        shifter -= 12;
    else if( shifter >= 28 && shifter < 32 )
        shifter -= 20;
    else
        return;

    if( !isNMI ) {
        EICBit = 1 << shifter;

        // Disable ISR and wake up
        if( EIC->INTENSET.reg & EICBit ) EIC->INTENCLR.reg &= EICBit;
        EIC->WAKEUP.reg &= ~EICBit;
        EIC->INTFLAG.reg |= EICBit;

        // Remove the callback from the ISR table
        ISRcallback[shifter] = 0;
    }
    else {
        ISRcallback[NUM_EXT_INTS] = 0;
        EIC->NMICTRL.reg = 0;
    }

    pinMode( pin, INPUT );
}

void EIC_Handler()
{
    uint32_t flags = ( EIC->INTFLAG.reg & EIC->INTENSET.reg );
    EIC->INTFLAG.reg = flags;

    // For each flag (starting highest to lowest priority) call the
    // corresponding function if it is not null
    for( uint32_t ptrNdx = 0; ptrNdx < NUM_EXT_INTS; ptrNdx++ ) {
        if( ( flags >> ptrNdx ) & 0x1 ) {
            if( ISRcallback[ptrNdx] != 0 ) {
                ISRcallback[ptrNdx]();
            }
        }
    }
}

void NMI_Handler()
{
    EIC->NMIFLAG.reg = EIC_NMIFLAG_NMI;
    ISRcallback[NUM_EXT_INTS]();
}
