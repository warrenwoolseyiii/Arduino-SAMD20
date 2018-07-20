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

int pinPeripheral( uint32_t ulPin, EPioType ulPeripheral )
{
    // Handle the case the pin isn't usable as PIO
    if( g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN ) return -1;

    // Ensure the APBBClk is enabled for PORT
    enableAPBBClk( PM_APBBMASK_PORT, 1 );
    switch( ulPeripheral ) {
        case PIO_DIGITAL:
        case PIO_INPUT:
        case PIO_INPUT_PULLUP:
        case PIO_OUTPUT:
            // Configure pin mode, if requested
            if( ulPeripheral == PIO_INPUT )
                pinMode( ulPin, INPUT );
            else {
                if( ulPeripheral == PIO_INPUT_PULLUP )
                    pinMode( ulPin, INPUT_PULLUP );
                else {
                    if( ulPeripheral == PIO_OUTPUT ) pinMode( ulPin, OUTPUT );
                }
            }
            break;

        case PIO_ANALOG:
        case PIO_SERCOM:
        case PIO_SERCOM_ALT:
        case PIO_TIMER:
        case PIO_TIMER_ALT:
        case PIO_EXTINT:
        case PIO_COM:
        case PIO_AC_CLK:
            // is pin odd?
            if( g_APinDescription[ulPin].ulPin & 1 ) {
                uint32_t temp;

                // Get whole current setup for both odd and even pins and remove
                // odd one
                temp = ( PORT->Group[g_APinDescription[ulPin].ulPort]
                             .PMUX[g_APinDescription[ulPin].ulPin >> 1]
                             .reg ) &
                       PORT_PMUX_PMUXE( 0xF );
                // Set new muxing
                PORT->Group[g_APinDescription[ulPin].ulPort]
                    .PMUX[g_APinDescription[ulPin].ulPin >> 1]
                    .reg = temp | PORT_PMUX_PMUXO( ulPeripheral );
                // Enable port mux
                PORT->Group[g_APinDescription[ulPin].ulPort]
                    .PINCFG[g_APinDescription[ulPin].ulPin]
                    .reg = PORT_PINCFG_PMUXEN;
            }
            // even pin
            else {
                uint32_t temp;

                temp = ( PORT->Group[g_APinDescription[ulPin].ulPort]
                             .PMUX[g_APinDescription[ulPin].ulPin >> 1]
                             .reg ) &
                       PORT_PMUX_PMUXO( 0xF );
                PORT->Group[g_APinDescription[ulPin].ulPort]
                    .PMUX[g_APinDescription[ulPin].ulPin >> 1]
                    .reg = temp | PORT_PMUX_PMUXE( ulPeripheral );
                PORT->Group[g_APinDescription[ulPin].ulPort]
                    .PINCFG[g_APinDescription[ulPin].ulPin]
                    .reg = PORT_PINCFG_PMUXEN; // Enable port mux
            }
            break;

        case PIO_NOT_A_PIN: return -1l; break;
    }

    return 0l;
}
