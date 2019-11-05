/*
  Copyright (c) 2014-2015 Arduino LLC.  All right reserved.

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
/* SAMD20E18
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * + Flume pin  +  E    Board pin  |  PIN   | Label/Name      | Comments (* is
 * for default peripheral in use)
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Digital Low      |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | PD0/RX     | 0 -> RX          |  PA25  | 0               | SERCOM3/PAD[3]
 * | PD1/TX     | 1 <- TX          |  PA24  | 1               | SERCOM3/PAD[2]
 * |            | 2                |  PA11  | 2               | GPIO
 * | PD6/FXS_RST| 3                |  PA14  | 3               | GPIO
 * | PD7/LEDRED | 4                |  PA15  | 4               | GPIO
 * | PB3/MOSI   | 5                |  PA16  | 5               | GPIO
 * | PB5/SCK    | 6                |  PA17  | 6               | GPIO
 * | PB2/RFM_SS | 7                |  PA18  | 7               | GPIO
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Digital High     |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | PB4/MISO   | 8                |  PA19  | 8               | GPIO
 * |            | 9                |  PA22  | 9               | GPIO
 * | PB0/FLSH_SS| 10               |  PA23  | 10              | GPIO
 * | FXOS_SS    | 11               |  PA27  | 11              | GPIO
 * | LEDGRN     | 12               |  PA28  | 12              | GPIO
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Analog Connector |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | PD4/INT1   | A0               |  PA02  | 13/A0           | AIN[0]  DAC/VOUT
 * | PD3/INT2   | A1               |  PA04  | 14/A1           | AIN[4]
 * | PD2/RFM_INT| A2               |  PA05  | 15/A2           | AIN[5]
 * | PC2/RFM_OSC| A3               |  PA06  | 16/A3           | AIN[6]
 * | PC1/RFM_RST| A4               |  PA07  | 17/A4           | AIN[7]
 * |            | A5               |  PA10  | 18/A5           | AIN[18]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Wire             |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | PC4/SDA    | SDA              |  PA08  | 19/SDA          | SERCOM0/PAD[0]
 * | PC5/SCL    | SCL              |  PA09  | 20/SCL          | SERCOM0/PAD[1]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            |SPI (Legacy ICSP) |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | RFM_MISO   | 1                |  PA19  | 21/MISO         | SERCOM1/PAD[3]
 * |            | 2                |        | 5V0             |
 * | RFM_MOSI   | 4                |  PA16  | 22/MOSI         | SERCOM1/PAD[0]
 * | RFM_SCK    | 3                |  PA17  | 23/SCK          | SERCOM1/PAD[1]
 * |            | 5                |        | RESET           |
 * |            | 6                |        | GND             |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            |SPI 2             |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | FXOS_MOSI  |                  |  PA08  | 24/MOSI         | SERCOM0/PAD[0]
 * | FXOS_SCK   |                  |  PA09  | 25/SCK          | SERCOM0/PAD[1]
 * | FXOS_MISO  |                  |  PA11  | 26/MISO         | SERCOM0/PAD[3]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Debug/Flashing   |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | RESET            | /RESET |                 |
 * |            | SWCLK            | PA30   |                 |
 * |            | SWDIO            | PA31   |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | AREF/DAC         |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | AREF             |  PA03  | 27              | AIN[1]
 * | PD4/INT1   | DAC/VOUT         |  PA02  | 28/A0           | AIN[0]  DAC/VOUT
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            |32.768KHz Crystal |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | PB6/TOSC1  |                  | PA00   | XIN32           |
 * | PB7/TOSC2  |                  | PA01   | XOUT32          |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 */

#include "variant.h"

// port, pin, extInt, analog, spi, i2c, uart, timer, clkOut
const ArduinoGPIO_t gArduinoPins[] = {
    // 0..12 - Digital pins
    // ----------------------
    // 0/1 - SERCOM/UART (Serial1)
    {PORTA, 25, PORT_PMUX_PMUXO_A, -1, -1, -1, PORT_PMUX_PMUXO_C, -1,
     -1}, // EXTInt, Analog, UART
    {PORTA, 24, PORT_PMUX_PMUXE_A, -1, -1, -1, PORT_PMUX_PMUXE_C, -1,
     -1}, // EXTInt, Analog, UART

    // 2..12
    // Digital Low
    {PORTA, 11, PORT_PMUX_PMUXO_A, PORT_PMUX_PMUXO_B, PORT_PMUX_PMUXO_C, -1, -1,
     PORT_PMUX_PMUXO_E, PORT_PMUX_PMUXO_H}, // EXTInt, Analog, SPI1, TC, GCLK
    {PORTA, 14, PORT_PMUX_PMUXE_A, -1, -1, -1, -1, PORT_PMUX_PMUXE_E,
     PORT_PMUX_PMUXE_H}, // EXTInt, TC, GCLK
    {PORTA, 15, PORT_PMUX_PMUXO_A, -1, -1, -1, -1, PORT_PMUX_PMUXO_E,
     PORT_PMUX_PMUXO_H}, // EXTInt, TC, GCLK
    {PORTA, 16, PORT_PMUX_PMUXE_A, PORT_PMUX_PMUXE_B, PORT_PMUX_PMUXE_C, -1, -1,
     PORT_PMUX_PMUXE_F, PORT_PMUX_PMUXE_H}, // EXTInt, Analog, SPI, TC, GCLK
    {PORTA, 17, PORT_PMUX_PMUXO_A, PORT_PMUX_PMUXO_B, PORT_PMUX_PMUXO_C, -1, -1,
     PORT_PMUX_PMUXO_F, PORT_PMUX_PMUXO_H}, // EXTInt, Analog, SPI, TC, GCLK
    {PORTA, 18, PORT_PMUX_PMUXE_A, PORT_PMUX_PMUXE_B, -1, -1, -1,
     PORT_PMUX_PMUXE_F, PORT_PMUX_PMUXE_H}, // EXTInt, Analog, TC, GCLK

    // Digital High
    {PORTA, 19, PORT_PMUX_PMUXO_A, PORT_PMUX_PMUXO_B, PORT_PMUX_PMUXO_C, -1, -1,
     PORT_PMUX_PMUXO_F, -1}, // EXTInt, Analog, SPI, TC
    {PORTA, 22, PORT_PMUX_PMUXE_A, PORT_PMUX_PMUXE_B, -1, -1, -1,
     PORT_PMUX_PMUXE_F, PORT_PMUX_PMUXE_H}, // EXTInt, Analog, TC, GCLK
    {PORTA, 23, PORT_PMUX_PMUXO_A, PORT_PMUX_PMUXO_B, -1, -1, -1,
     PORT_PMUX_PMUXO_F, PORT_PMUX_PMUXO_H}, // EXTInt, Analog, TC, GCLK
    {PORTA, 27, PORT_PMUX_PMUXO_A, -1, -1, -1, -1, -1,
     PORT_PMUX_PMUXO_H}, // EXTInt, GCLK
    {PORTA, 28, PORT_PMUX_PMUXE_A, -1, -1, -1, -1, -1,
     PORT_PMUX_PMUXE_H}, // EXTInt, GCLK

    // 13..18 - Analog pins
    // --------------------
    {PORTA, 2, PORT_PMUX_PMUXE_A, PORT_PMUX_PMUXE_B, -1, -1, -1, -1,
     -1}, // EXTInt, Analog
    {PORTA, 4, PORT_PMUX_PMUXE_A, PORT_PMUX_PMUXE_B, -1, -1, -1,
     PORT_PMUX_PMUXE_F, -1}, // EXTInt, Analog, TC
    {PORTA, 5, PORT_PMUX_PMUXO_A, PORT_PMUX_PMUXO_B, -1, -1, -1,
     PORT_PMUX_PMUXO_F, -1}, // EXTInt, Analog, TC
    {PORTA, 6, PORT_PMUX_PMUXE_A, PORT_PMUX_PMUXE_B, -1, -1, -1,
     PORT_PMUX_PMUXE_F, -1}, // EXTInt, Analog, TC
    {PORTA, 7, PORT_PMUX_PMUXO_A, PORT_PMUX_PMUXO_B, -1, -1, -1,
     PORT_PMUX_PMUXO_F, -1}, // EXTInt, Analog, TC
    {PORTA, 10, PORT_PMUX_PMUXE_A, PORT_PMUX_PMUXE_B, -1, -1, -1,
     PORT_PMUX_PMUXE_E, PORT_PMUX_PMUXE_H}, // EXTInt, Analog, TC, GCLK

    // 19..20 I2C pins (SDA/SCL)
    // ----------------------
    {PORTA, 8, PORT_PMUX_PMUXE_A, PORT_PMUX_PMUXE_B, PORT_PMUX_PMUXE_C,
     PORT_PMUX_PMUXE_C, -1, PORT_PMUX_PMUXE_E,
     -1}, // EXTInt, Analog, SPI1, I2C, TC
    {PORTA, 9, PORT_PMUX_PMUXO_A, PORT_PMUX_PMUXO_B, PORT_PMUX_PMUXO_C,
     PORT_PMUX_PMUXO_C, -1, PORT_PMUX_PMUXO_E,
     -1}, // EXTInt, Analog, SPI1, I2C, TC

    // 21 (AREF)
    {PORTA, 3, PORT_PMUX_PMUXO_A, PORT_PMUX_PMUXO_B, -1, -1, -1, -1,
     -1} // EXTInt, Analog
};

// TODO: See if we need this
const void *g_apTCInstances[TC_INST_NUM] = {TC0, TC1, TC2, TC3, TC4, TC5};

// Sercom objects
SERCOM sercom0( SERCOM0 );
SERCOM sercom1( SERCOM1 );
SERCOM sercom2( SERCOM2 );
SERCOM sercom3( SERCOM3 );

Uart Serial( &sercom3, PIN_SERIAL_RX, PIN_SERIAL_TX, PAD_SERIAL_RX,
             PAD_SERIAL_TX );

void SERCOM3_Handler()
{
    Serial.IrqHandler();
}

// Timer counter objects
TimerCounter Timer( TC2 );
TimerCounter Timer1( TC3 );
TimerCounter Timer2( TC4 );
TimerCounter Timer3( TC5 );

// PWM objects
PWM PWMChannel0( &Timer );
PWM PWMChannel1( &Timer1 );
PWM PWMChannel2( &Timer2 );
PWM PWMChannel3( &Timer3 );

void TC2_Handler()
{
    Timer.IrqHandler();
}

void TC3_Handler()
{
    Timer1.IrqHandler();
}

void TC4_Handler()
{
    Timer2.IrqHandler();
}

void TC5_Handler()
{
    Timer3.IrqHandler();
}

// Emulated EEPROM
EEEPROM EEPROM;
