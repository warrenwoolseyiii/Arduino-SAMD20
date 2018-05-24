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
/*
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

#include "variant.h"

/*
 * Pins descriptions
 */
const PinDescription g_APinDescription[]=
{
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
} ;

const void* g_apTCInstances[0+TC_INST_NUM]={ TC0, TC1, TC2, TC3, TC4, TC5 } ;

// Multi-serial objects instantiation
SERCOM sercom0( SERCOM0 ) ;
SERCOM sercom1( SERCOM1 ) ;
SERCOM sercom2( SERCOM2 ) ;
SERCOM sercom3( SERCOM3 ) ;
//SERCOM sercom4( SERCOM4 ) ;
//SERCOM sercom5( SERCOM5 ) ;

Uart Serial( &sercom3, PIN_SERIAL_RX, PIN_SERIAL_TX, PAD_SERIAL_RX, PAD_SERIAL_TX ) ;
void SERCOM3_Handler()
{
  Serial.IrqHandler();
}


