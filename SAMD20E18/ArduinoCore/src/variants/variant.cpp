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
 * + Flume pin  +  E    Board pin  |  PIN   | Label/Name      | Comments (* is for default peripheral in use)
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Digital Low      |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | PD0/RX     | 0 -> RX          |  PA25  | 0               | EIC/EXTINT[13]                                 *SERCOM3/PAD[3]  SERCOM5/PAD[3]  TC5/WO[1]
 * | PD1/TX     | 1 <- TX          |  PA24  | 1               | EIC/EXTINT[12]                                 *SERCOM3/PAD[2]  SERCOM5/PAD[2]  TC5/WO[0]
 * |            | 2                |  PA11  | 2               | EIC/EXTINT[11]  ADC/AIN[19]          PTC/X[3]   SERCOM0/PAD[3]  SERCOM2/PAD[3]  TC1/WO[1]    
 * | PD6/FXS_RST| 3                |  PA14  | 3               | EIC/EXTINT[14]                                  SERCOM2/PAD[2]  SERCOM4/PAD[2]  TC3/WO[0]
 * | PD7/LED1   | 4                |  PA15  | 4               | EIC/EXTINT[15]                                  SERCOM2/PAD[3]  SERCOM4/PAD[3]  TC3/WO[1]
 * | PB3/MOSI   | 5                |  PA16  | 5               | EIC/EXTINT[0]                        PTC/X[4]   SERCOM1/PAD[0]  SERCOM3/PAD[0]  TC2/WO[0]
 * | PB5/SCK    | 6                |  PA17  | 6               | EIC/EXTINT[1]                        PTC/X[5]   SERCOM1/PAD[1]  SERCOM3/PAD[1]  TC2/WO[1]
 * | PB2/RFM_SS | 7                |  PA18  | 7               | EIC/EXTINT[2]                        PTC/X[6]   SERCOM1/PAD[2]  SERCOM3/PAD[2]  TC3/WO[0]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Digital High     |        |                 |
 * +------------+------------------+--------+-----------------+-------------------------------------------------------------------------------------------------------- 
 * | PB4/MISO   | 8                |  PA19  | 8               | EIC/EXTINT[3]                        PTC/X[7]   SERCOM1/PAD[3]  SERCOM3/PAD[3] TC3/WO[1]
 * | PB1/LED2   | 9                |  PA22  | 9               | EIC/EXTINT[6]                        PTC/X[10]  SERCOM3/PAD[0]  SERCOM5/PAD[0] TC4/WO[0] 
 * | PB0/FLSH_SS| 10               |  PA23  | 10              | EIC/EXTINT[7]                        PTC/X[11]  SERCOM3/PAD[1]  SERCOM5/PAD[1] TC4/WO[1] 
 * |            | 11               |  PA27  | 11              | EIC/EXTINT[15]
 * |            | 12               |  PA28  | 12              | EIC/EXTINT[8]
 * |            | 13               |  NC    |                 |         
 * |            | GND              |        |                 |
 * |            | AREF             |  PA03  | 13              | EIC/EXTINT[3] *[ADC|DAC]/VREFA ADC/AIN[1] PTC/Y[1]
 * | PC4/SDA    | SDA              |  PA08  | 14/SDA          | EIC/NMI        ADC/AIN[16]           PTC/X[0]  *SERCOM0/PAD[0]  SERCOM2/PAD[0] TC0/WO[0]
 * | PC5/SCL    | SCL              |  PA09  | 15/SCL          | EIC/EXTINT[7]                        PTC/X[1]  *SERCOM3/PAD[1]  SERCOM2/PAD[1] TC0/WO[1]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Analog Connector |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | PD4/INT1   | A0               |  PA02  | 16/A0           | EIC/EXTINT[2] *ADC/AIN[0]  DAC/VOUT  PTC/Y[0]
 * | PD3/INT2   | A1               |  PA04  | 17/A1           | EIC/EXTINT[4] *ADC/AIN[4]  AC/AIN[0] PTC/Y[2]  SERCOM0/PAD[0]                  TC0/WO[0]
 * | PD2/RFM_INT| A2               |  PA05  | 18/A2           | EIC/EXTINT[5] *ADC/AIN[5]  AC/AIN[1] PTC/Y[3]  SERCOM0/PAD[1]                  TC0/WO[1]
 * | PC2/RFM_OSC| A3               |  PA06  | 19/A3           | EIC/EXTINT[6] *ADC/AIN[6]  AC/AIN[2] PTC/Y[4]  SERCOM0/PAD[2]                  TC1/WO[0]
 * | PC1/RFM_RST| A4               |  PA07  | 20/A4           | EIC/EXTINT[7] *ADC/AIN[7]  AC/AIN[3] PTC/Y[5]  SERCOM0/PAD[3]                  TC1/WO[1]
 * |            | A5               |  PA10  | 21/A5           | EIC/EXTINT[10]*ADC/AIN[18]           PTC/X[12  SERCOM0/PAD[2]  SERCOM2/PAD[2]  TC1/WO[0]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------

 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Debug/Flashing   |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | RESET            | /RESET |                 |
 * |            | SWCLK            | PA30   |                 |
 * |            | SWDIO            | PA31   |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            |32.768KHz Crystal |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | PB6/TOSC1  |                  | PA00   | XIN32           | EIC/EXTINT[0] SERCOM1/PAD[0] TCC2/WO[0]
 * | PB7/TOSC2  |                  | PA01   | XOUT32          | EIC/EXTINT[1] SERCOM1/PAD[1] TCC2/WO[1]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 */

/* SAMD20J18
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * + Pin number +  E    Board pin  |  PIN   | Label/Name      | Comments (* is for default peripheral in use)
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Digital Low      |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 0          | 0 -> RX          |  PB13  | RX/SDA1         | SERCOM4/PAD[1]
 * | 1          | 1 <- TX          |  PB12  | TX/SCL1         | SERCOM4/PAD[0]
 * | 2          | 2                |  PB06  |                 | GPIO
 * | 3          | 3                |  PB07  |                 | GPIO
 * | 4          | 4                |  PB04  |                 | EXTINT[4]
 * | 5          | 5                |  PB05  |                 | GPIO
 * | 6          | 6                |  PA20  |                 | GPIO
 * | 7          | 7                |  PA21  |                 | GPIO
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Digital High     |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 8          | 8                |  PB15  |                 | GPIO
 * | 9          | 9                |  PB30  |                 | GPIO 
 * | 10         | 10               |  PA28  |                 | EXTINT[8]
 * | 11         | 11               |  PB22  | MOSI            | SERCOM5/PAD[2]
 * | 12         | 12               |  PB16  | MISO            | SERCOM5/PAD[0]
 * | 13         | 13               |  PB23  | SCK             | SERCOM5/PAD[3] 
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Analog Connector |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 14         | A0               |  PB00  | A0              | AIN[8]
 * | 15         | A1               |  PB01  | A1              | AIN[9]
 * | 16         | A2               |  PA10  | A2              | AIN[18]
 * | 17         | A3               |  PA11  | A3              | AIN[19] 
 * | 18         | A4               |  PA02  | A4              | AIN[0]
 * | 19         | A5               |  PA03  | A5              | AIN[1]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | Wire             |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 20         | SDA              |  PA08  | SDA             | SERCOM2/PAD[0]
 * | 21         | SCL              |  PA09  | SCL             | SERCOM2/PAD[1]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            |SPI (Legacy ICSP) |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 22         | 1                |  PA04  | MISO            | SERCOM0/PAD[0]
 * |            | 2                |        | 5V0             |
 * | 23         | 4                |  PA06  | MOSI            | SERCOM0/PAD[2]
 * | 24         | 3                |  PA07  | SCK             | SERCOM0/PAD[3]
 * |            | 5                |        | RESET           |
 * |            | 6                |        | GND             |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | LEDs             |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 25         |                  |  PA14  | LED0            | GPIO
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | EDBG             |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | EDBG_SWD         |  PA30  | SWD Clock       |
 * |            |                  |  PA31  | SWD Data        |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 26         | EDBG_UART        |  PA24  | EDBG_UART TX    | *SERCOM3/PAD[2]
 * | 27         |                  |  PA25  | EDBG_UART RX    | *SERCOM3/PAD[3]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 28         | EDBG_I2C         |  PA08  | EDBG_SDA        | SERCOM2/PAD[0]
 * | 29         |                  |  PA09  | EDBG_SCL        | SERCOM2/PAD[1]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 30         | EDBG_SPI         |  PB16  | EDBG_MISO       | SERCOM5/PAD[0]
 * | 31         |                  |  PB22  | EDBG_MOSI       | SERCOM5/PAD[2]
 * | 32         |                  |  PB31  | EDBG_SS         | SERCOM5/PAD[1]
 * | 33         |                  |  PB23  | EDBG_SCK        | SERCOM5/PAD[3]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * | 34         | EDBG_IO          |  PA27  | EDBG_GPIO0      | GPIO
 * | 35         |                  |  PA28  | EDBG_GPIO1      | GPIO
 * | 36         |                  |  PA20  | EDBG_GPIO2      | GPIO
 * | 37         |                  |  PA21  | EDBG_GPIO3      | GPIO
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            |                  |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            | GND              |        |                 |
 * | 38         | AREF             |  PA03  |                 | AIN[1]
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            |32.768KHz Crystal |        |                 |
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 * |            |                  |  PA00  | XIN32           | 
 * |            |                  |  PA01  | XOUT32          | 
 * +------------+------------------+--------+-----------------+--------------------------------------------------------------------------------------------------------
 */

#include "variant.h"

/*
 * Pins descriptions
 */
const PinDescription g_APinDescription[]=
{
#if defined(__SAMD20E18__)
  // 0..12 - Digital pins
  // ----------------------
  // 0/1 - SERCOM/UART (Serial1)
  { PORTA, 25, PIO_SERCOM, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_13 }, // RX: SERCOM3/PAD[3]
  { PORTA, 24, PIO_SERCOM, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_12 }, // TX: SERCOM3/PAD[2]

  // 2..12
  // Digital Low
  { PORTA, 11, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM1_CH1, TCC1_CH1, EXTERNAL_INT_11 }, //TC1/WO[1]
  { PORTA, 14, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM3_CH0, TC3_CH0, EXTERNAL_INT_14 }, // TC3/WO[0]
  { PORTA, 15, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM3_CH1, TC3_CH1, EXTERNAL_INT_15 },  // TC3/WO[1]
  { PORTA, 16, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM2_CH0, TCC2_CH0, EXTERNAL_INT_0 }, // TC2/WO[0]
  { PORTA, 17, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM2_CH1, TCC2_CH1, EXTERNAL_INT_1 }, // TC2/WO[1]
  { PORTA, 18, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_2 },

  // Digital High
  { PORTA, 19, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_3 },
  { PORTA, 22, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM4_CH0, TC4_CH0, EXTERNAL_INT_6 }, // TC4/WO[0]
  { PORTA, 23, PIO_TIMER, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM4_CH1, TC4_CH1, EXTERNAL_INT_7 }, // TC4/WO[1]
  { PORTA, 27, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_15 },
  { PORTA, 28, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_8 },

  // 13..18 - Analog pins
  // --------------------
  { PORTA,  2, PIO_ANALOG, (PIN_ATTR_ANALOG|PIN_ATTR_DIGITAL), ADC_Channel0, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_2 }, // ADC/AIN[0]
  { PORTA,  4, PIO_ANALOG, (PIN_ATTR_ANALOG|PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), ADC_Channel4, PWM0_CH0, TCC0_CH0, EXTERNAL_INT_4 }, // ADC/AIN[4]
  { PORTA,  5, PIO_ANALOG, (PIN_ATTR_ANALOG|PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), ADC_Channel5, PWM0_CH1, TCC0_CH1, EXTERNAL_INT_5 }, // ADC/AIN[5]
  { PORTA,  6, PIO_ANALOG, (PIN_ATTR_ANALOG|PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), ADC_Channel6, PWM1_CH0, TCC1_CH0, EXTERNAL_INT_6 }, // ADC/AIN[6]
  { PORTA,  7, PIO_ANALOG, (PIN_ATTR_ANALOG|PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), ADC_Channel7, PWM1_CH1, TCC1_CH1, EXTERNAL_INT_7 }, // ADC/AIN[7]
  { PORTA, 10, PIO_ANALOG, (PIN_ATTR_ANALOG|PIN_ATTR_DIGITAL), ADC_Channel18, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_10 }, // ADC/AIN[18]

  // 19..20 I2C pins (SDA/SCL)
  // ----------------------
  { PORTA,  8, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NMI }, // SDA: SERCOM0/PAD[0]
  { PORTA,  9, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_9 }, // SCL: SERCOM0/PAD[1]

  // 21..23 - SPI pins (ICSP:MISO,SCK,MOSI)
  // ----------------------
  { PORTA, 19, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_3 }, // MISO: SERCOM1/PAD[0]
  { PORTA, 16, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_0 }, // MOSI: SERCOM1/PAD[2]
  { PORTA, 17, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_1 }, // SCK: SERCOM1/PAD[3]

  // 24 (AREF)
  { PORTA, 3, PIO_ANALOG, PIN_ATTR_ANALOG, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // DAC/VREFP

  // ----------------------
  // 25 - Alternate use of A0 (DAC output)
  { PORTA, 2, PIO_ANALOG, PIN_ATTR_ANALOG, DAC_Channel0, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_2 }, // DAC/VOUT  
#elif defined(__SAMD20J18__)
  // 0..13 - Digital pins
  // ----------------------
  // 0/1 - SERCOM/UART (Serial1)
  { PORTB, 13, PIO_SERCOM, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_13 }, // RX: SERCOM4/PAD[1]
  { PORTB, 12, PIO_SERCOM, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_12 }, // TX: SERCOM4/PAD[0]

  // 2..13
  // Digital Low
  { PORTB,  6, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_6 },
  { PORTB,  7, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_7 },
  { PORTB,  4, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_4 },
  { PORTB,  5, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_5 },
  { PORTA, 20, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM0_CH6, TCC0_CH6, EXTERNAL_INT_4 }, // TC7/WO[0]
  { PORTA, 21, PIO_DIGITAL, (PIN_ATTR_DIGITAL|PIN_ATTR_PWM|PIN_ATTR_TIMER), No_ADC_Channel, PWM0_CH7, TCC0_CH7, EXTERNAL_INT_5 }, // TC7/WO[1]

  // Digital High
  { PORTB, 15, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_15 },
  { PORTB, 30, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_14 },
  { PORTA, 28, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_8 },
  { PORTB, 22, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_6 },
  { PORTB, 16, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_0 },
  { PORTB, 12, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_12 },

  // 14..19 - Analog pins
  // --------------------
  { PORTB,  0, PIO_ANALOG, PIN_ATTR_ANALOG, ADC_Channel8, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_0 }, // ADC/AIN[8]
  { PORTB,  1, PIO_ANALOG, PIN_ATTR_ANALOG, ADC_Channel9, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_1 }, // ADC/AIN[9]
  { PORTA, 10, PIO_ANALOG, PIN_ATTR_ANALOG, ADC_Channel18, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_10 }, // ADC/AIN[18]
  { PORTA, 11, PIO_ANALOG, PIN_ATTR_ANALOG, ADC_Channel19, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_11 }, // ADC/AIN[19]
  { PORTA,  2, PIO_ANALOG, PIN_ATTR_ANALOG, ADC_Channel0, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_2 }, // ADC/AIN[0]
  { PORTB,  3, PIO_ANALOG, PIN_ATTR_ANALOG, ADC_Channel1, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_3 }, // ADC/AIN[1]

  // 20..21 I2C pins (SDA/SCL and also EDBG:SDA/SCL)
  // ----------------------
  { PORTA,  8, PIO_SERCOM_ALT, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NMI }, // SDA: SERCOM2/PAD[0]
  { PORTA,  9, PIO_SERCOM_ALT, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_9 }, // SCL: SERCOM2/PAD[1]

  // 22..24 - SPI pins (ICSP:MISO,SCK,MOSI)
  // ----------------------
  { PORTA,  4, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_12 }, // MISO: SERCOM0/PAD[0]
  { PORTA,  6, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_10 }, // MOSI: SERCOM0/PAD[2]
  { PORTA,  7, PIO_SERCOM, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_11 }, // SCK: SERCOM40/PAD[3]

  // 25 LED
  // --------------------
  { PORTA, 14, PIO_OUTPUT, PIN_ATTR_DIGITAL, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // used as output only

  // 26..37 - EDBG
  // ----------------------
  // 26/27 - EDBG/UART
  { PORTA, 24, PIO_SERCOM_ALT, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // TX: SERCOM3/PAD[2]
  { PORTA, 25, PIO_SERCOM_ALT, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // RX: SERCOM3/PAD[3]

  // 28/29 I2C (SDA/SCL and also EDBG:SDA/SCL)
  { PORTA,  8, PIO_SERCOM, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // SDA: SERCOM2/PAD[0]
  { PORTA,  9, PIO_SERCOM, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // SCL: SERCOM2/PAD[1]

  // 30..33 - EDBG/SPI
  { PORTB, 16, PIO_SERCOM, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // MISO: SERCOM5/PAD[0]
  { PORTB, 22, PIO_SERCOM_ALT, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // MOSI: SERCOM5/PAD[2]
  { PORTB, 31, PIO_SERCOM, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // SS: SERCOM5/PAD[1]
  { PORTB, 23, PIO_SERCOM_ALT, PIN_ATTR_NONE, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // SCK: SERCOM5/PAD[3]

  // 34..37 - EDBG/Digital
  { PORTA, 27, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, 
  { PORTA, 28, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, 
  { PORTA, 20, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, 
  { PORTA, 21, PIO_DIGITAL, (PIN_ATTR_DIGITAL), No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, 

  // 38 (AREF)
  { PORTA, 3, PIO_ANALOG, PIN_ATTR_ANALOG, No_ADC_Channel, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_NONE }, // DAC/VREFP

  // ----------------------
  // 39 - Alternate use of A0 (DAC output)
  { PORTA,  2, PIO_ANALOG, PIN_ATTR_ANALOG, DAC_Channel0, NOT_ON_PWM, NOT_ON_TIMER, EXTERNAL_INT_2 }, // DAC/VOUT
#endif
} ;

#if defined(__SAMD20E18__)
const void* g_apTCInstances[TC_INST_NUM]={ TC0, TC1, TC2, TC3, TC4, TC5 } ;
#elif defined(__SAMD20J18__)
const void* g_apTCInstances[TC_INST_NUM]={ TC0, TC1, TC2, TC3, TC4, TC5, TC6, TC7 } ;
#endif

// Multi-serial objects instantiation
#if (defined(__SAMD20E18__) || defined(__SAMD20J18__))
SERCOM sercom0( SERCOM0 ) ;
SERCOM sercom1( SERCOM1 ) ;
SERCOM sercom2( SERCOM2 ) ;
SERCOM sercom3( SERCOM3 ) ;
#if defined(__SAMD20J18__)
SERCOM sercom4( SERCOM4 ) ;
SERCOM sercom5( SERCOM5 ) ;
#endif /* __SAMD20J18__ */
#endif 

#if defined(__SAMD20E18__)
Uart Serial( &sercom3, PIN_SERIAL_RX, PIN_SERIAL_TX, PAD_SERIAL_RX, PAD_SERIAL_TX ) ;
void SERCOM3_Handler()
{
  Serial.IrqHandler();
}
#elif defined(__SAMD20J18__)
Uart Serial1( &sercom4, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PAD_SERIAL1_RX, PAD_SERIAL1_TX ) ;
Uart Serial( &sercom3, PIN_SERIAL_RX, PIN_SERIAL_TX, PAD_SERIAL_RX, PAD_SERIAL_TX ) ;

void SERCOM4_Handler()
{
  Serial1.IrqHandler();
}
  
void SERCOM3_Handler()
{
  Serial.IrqHandler();
}

#endif


