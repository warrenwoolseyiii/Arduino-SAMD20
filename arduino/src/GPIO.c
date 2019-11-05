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
#include "GPIO.h"
#include "Arduino.h"

// Helper macros for common operations
#define SET_INPUT( port, pin ) ( PORT->Group[port].DIR.reg &= ~( 1 << pin ) )
#define SET_OUTPUT( port, pin ) ( PORT->Group[port].DIR.reg |= ( 1 << pin ) )
#define SET_OUT_LOW( port, pin ) ( PORT->Group[port].OUT.reg &= ~( 1 << pin ) )
#define SET_OUT_HIGH( port, pin ) ( PORT->Group[port].OUT.reg |= ( 1 << pin ) )
#define ENABLE_READS( port, pin ) \
    ( PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_INEN )
#define DISABLE_READS( port, pin ) \
    ( PORT->Group[port].PINCFG[pin].reg &= ~( PORT_PINCFG_INEN ) )

void pinMode( uint32_t ulPin, uint32_t ulMode )
{
    enableAPBBClk( PM_APBBMASK_PORT, 1 );

    if( ulMode & INPUT ) {
        uint32_t pinCfg = 0;
        SET_INPUT( gArduinoPins[ulPin].port, gArduinoPins[ulPin].pin );

        // Check to see if we are using a pull-up or pull-down, if so enable the
        // pull-x bit and set the OUT register accordingly
        ulMode &= ~( INPUT );
        if( ulMode & ( INPUT_PULLDOWN | INPUT_PULLUP ) ) {
            pinCfg |= PORT_PINCFG_PULLEN;
            if( ulMode & INPUT_PULLDOWN )
                SET_OUT_LOW( gArduinoPins[ulPin].port,
                             gArduinoPins[ulPin].pin );
            else
                SET_OUT_HIGH( gArduinoPins[ulPin].port,
                              gArduinoPins[ulPin].pin );
        }

        // Set pin configuration register
        PORT->Group[gArduinoPins[ulPin].port]
            .PINCFG[gArduinoPins[ulPin].pin]
            .reg = pinCfg;
    }
    else if( ulMode & TRI_STATE ) {
        SET_INPUT( gArduinoPins[ulPin].port, gArduinoPins[ulPin].pin );
        SET_OUT_LOW( gArduinoPins[ulPin].port, gArduinoPins[ulPin].pin );
        PORT->Group[gArduinoPins[ulPin].port]
            .PINCFG[gArduinoPins[ulPin].pin]
            .reg = 0;
    }
    else if( ulMode & OUTPUT ) {
        SET_OUT_LOW( gArduinoPins[ulPin].port, gArduinoPins[ulPin].pin );
        SET_OUTPUT( gArduinoPins[ulPin].port, gArduinoPins[ulPin].pin );
        PORT->Group[gArduinoPins[ulPin].port]
            .PINCFG[gArduinoPins[ulPin].pin]
            .reg = 0;
    }
    else {
        uint32_t pmux;

        // Configure for peripheral, ensure direction is cleared
        ulMode &= ~( GPIO_FUNC_MASK );

        // Pin is odd
        if( gArduinoPins[ulPin].pin & 0x1 ) {
            // Read and preserve the even bits, then set the odd bits
            pmux = PORT->Group[gArduinoPins[ulPin].port]
                       .PMUX[gArduinoPins[ulPin].pin >> 1]
                       .reg;
            pmux &= ~( PORT_PMUX_PMUXO_Msk );
            pmux |= ulMode;
            PORT->Group[gArduinoPins[ulPin].port]
                .PMUX[gArduinoPins[ulPin].pin >> 1]
                .reg = pmux;
        }
        else {
            // Read and preserve the odd bits, then set the even bits
            pmux = PORT->Group[gArduinoPins[ulPin].port]
                       .PMUX[gArduinoPins[ulPin].pin >> 1]
                       .reg;
            pmux &= ~( PORT_PMUX_PMUXE_Msk );
            pmux |= ulMode;
            PORT->Group[gArduinoPins[ulPin].port]
                .PMUX[gArduinoPins[ulPin].pin >> 1]
                .reg = pmux;
        }

        // Enable the pin for peripheral function
        PORT->Group[gArduinoPins[ulPin].port]
            .PINCFG[gArduinoPins[ulPin].pin]
            .reg = PORT_PINCFG_PMUXEN;
    }
}

void digitalWrite( uint32_t dwPin, uint32_t dwVal )
{
    enableAPBBClk( PM_APBBMASK_PORT, 1 );

    // Ensure the pin is output, otherwise make it an output
    if( !( PORT->Group[gArduinoPins[dwPin].port].DIR.reg &
           ( 1 << gArduinoPins[dwPin].pin ) ) )
        pinMode( dwPin, OUTPUT );

    if( dwVal )
        SET_OUT_HIGH( gArduinoPins[dwPin].port, gArduinoPins[dwPin].pin );
    else
        SET_OUT_LOW( gArduinoPins[dwPin].port, gArduinoPins[dwPin].pin );
}

uint8_t digitalRead( uint32_t ulPin )
{
    uint8_t rtn = 0;

    enableAPBBClk( PM_APBBMASK_PORT, 1 );

    // To conserve power we only enable digital sampling when we request a read,
    // so enable digital sampling, read the pin and then disable it
    ENABLE_READS( gArduinoPins[ulPin].port, gArduinoPins[ulPin].pin );
    rtn = ( PORT->Group[gArduinoPins[ulPin].port].IN.reg &
            ( 1 << gArduinoPins[ulPin].pin ) ) > 0;
    DISABLE_READS( gArduinoPins[ulPin].port, gArduinoPins[ulPin].pin );
    return rtn;
}
