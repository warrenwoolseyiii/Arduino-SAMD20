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
#include "wiring_private.h"

#include <string.h>

#define EIC_WAIT_SYNC while( EIC->STATUS.bit.SYNCBUSY )

static voidFuncPtr ISRcallback[EXTERNAL_NUM_INTERRUPTS];
static uint32_t    ISRlist[EXTERNAL_NUM_INTERRUPTS];

uint32_t _numRegisteredISRs;
uint8_t  _enabled = 0;
uint8_t  _lowPowerModeActive = 0;

static void __initialize( uint8_t lowPower )
{
    uint32_t clkSrc = GCLK_CLKCTRL_GEN_GCLK0_Val;

    memset( ISRlist, 0, sizeof( ISRlist ) );
    memset( ISRcallback, 0, sizeof( ISRcallback ) );
    _numRegisteredISRs = 0;

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
void attachInterrupt( uint32_t pin, voidFuncPtr callback, uint32_t mode )
{
    uint32_t config;
    uint32_t pos;

#if ARDUINO_SAMD_VARIANT_COMPLIANCE >= 10606
    EExt_Interrupts in = g_APinDescription[pin].ulExtInt;
#else
    EExt_Interrupts in = digitalPinToInterrupt( pin );
#endif
    if( in == NOT_AN_INTERRUPT || in == EXTERNAL_INT_NMI ) return;

    if( !_enabled ) __initialize( 0 );

    // Enable wakeup capability on pin in case being used during sleep
    uint32_t inMask = 1 << in;
    EIC->WAKEUP.reg |= inMask;

    // Assign pin to EIC
    pinPeripheral( pin, PIO_EXTINT );

    // Only store when there is really an ISR to call.
    // This allow for calling attachInterrupt(pin, NULL, mode), we set up all
    // needed register but won't service the interrupt, this way we also don't
    // need to check it inside the ISR.
    if( callback ) {
        // Store interrupts to service in order of when they were attached
        // to allow for first come first serve handler
        uint32_t current = 0;

        // Check if we already have this interrupt
        for( current = 0; current < _numRegisteredISRs; current++ ) {
            if( ISRlist[current] == inMask ) {
                break;
            }
        }
        if( current == _numRegisteredISRs ) {
            // Need to make a new entry
            _numRegisteredISRs++;
        }
        ISRlist[current] =
            inMask; // List of interrupt in order of when they were attached
        ISRcallback[current] = callback; // List of callback adresses

        // Look for right CONFIG register to be addressed
        if( in > EXTERNAL_INT_7 ) {
            config = 1;
            pos = ( in - 8 ) << 2;
        }
        else {
            config = 0;
            pos = in << 2;
        }

        // Configure the interrupt mode
        EIC->CONFIG[config].reg &=
            ~( EIC_CONFIG_SENSE0_Msk << pos ); // Reset sense mode, important
                                               // when changing trigger mode
                                               // during runtime
        switch( mode ) {
            case LOW:
                EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_LOW_Val << pos;
                break;
            case HIGH:
                EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_HIGH_Val << pos;
                break;
            case CHANGE:
                EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_BOTH_Val << pos;
                break;
            case FALLING:
                EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_FALL_Val << pos;
                break;
            case RISING:
                EIC->CONFIG[config].reg |= EIC_CONFIG_SENSE0_RISE_Val << pos;
                break;
        }
    }
    // Enable the interrupt
    EIC->INTENSET.reg = EIC_INTENSET_EXTINT( inMask );
}

// Disables the selected pin from external interrupt control and removes
// the callback associated with that pin.
void detachInterrupt( uint32_t pin )
{
#if( ARDUINO_SAMD_VARIANT_COMPLIANCE >= 10606 )
    EExt_Interrupts in = g_APinDescription[pin].ulExtInt;
#else
    EExt_Interrupts in = digitalPinToInterrupt( pin );
#endif
    if( in == NOT_AN_INTERRUPT || in == EXTERNAL_INT_NMI ) return;

    uint32_t inMask = 1 << in;
    EIC->INTENCLR.reg = EIC_INTENCLR_EXTINT( inMask );

    // Disable wakeup capability on pin during sleep
    EIC->WAKEUP.reg &= ~inMask;

    // Remove callback from the ISR list
    uint32_t current;
    for( current = 0; current < _numRegisteredISRs; current++ ) {
        if( ISRlist[current] == inMask ) {
            break;
        }
    }
    if( current == _numRegisteredISRs ) return; // We didn't have it

    // Shift the reminder down
    for( ; current < _numRegisteredISRs - 1; current++ ) {
        ISRlist[current] = ISRlist[current + 1];
        ISRcallback[current] = ISRcallback[current + 1];
    }
    _numRegisteredISRs--;
}

void EIC_Handler()
{
    // Calling the routine directly from -here- takes about 1us
    // Depending on where you are in the list it will take longer

    // Loop over all enabled interrupts in the list
    for( uint32_t i = 0; i < _numRegisteredISRs; i++ ) {
        if( ( EIC->INTFLAG.reg & ISRlist[i] ) != 0 ) {
            // Call the callback function
            ISRcallback[i]();
            // Clear the interrupt
            EIC->INTFLAG.reg = ISRlist[i];
        }
    }
}
