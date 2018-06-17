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

#ifndef _VARIANT_ARDUINO_E_
#define _VARIANT_ARDUINO_E_

// The definitions here needs a SAMD core >=1.6.10
#define ARDUINO_SAMD_VARIANT_COMPLIANCE 10610

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/

/** Frequency of the board main oscillator */
#define VARIANT_MAINOSC		(32768ul)

/** Master clock frequency */
#define VARIANT_MCK			  (48000000ul)

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
#include "SERCOM.h"
#include "Uart.h"
#include "TimerCounter.h"
#endif // __cplusplus

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/*----------------------------------------------------------------------------
 *        Pins
 *----------------------------------------------------------------------------*/

// Number of pins defined in PinDescription array
#if defined(__SAMD20E18__)
#define PINS_COUNT           (22u)
#define NUM_DIGITAL_PINS     (16u)
#define NUM_ANALOG_INPUTS    (6u)
#define NUM_ANALOG_OUTPUTS   (1u)
#define analogInputToDigitalPin(p)  ((p < 6u) ? (p) + 14u : -1)
#elif defined(__SAMD20J18__)
#define PINS_COUNT           (26u)
#define NUM_DIGITAL_PINS     (20u)
#define NUM_ANALOG_INPUTS    (6u)
#define NUM_ANALOG_OUTPUTS   (1u)
#define analogInputToDigitalPin(p)  ((p < 6u) ? (p) + 14u : -1)
#endif

#define digitalPinToPort(P)        ( &(PORT->Group[g_APinDescription[P].ulPort]) )
#define digitalPinToBitMask(P)     ( 1 << g_APinDescription[P].ulPin )
//#define analogInPinToBit(P)        ( )
#define portOutputRegister(port)   ( &(port->OUT.reg) )
#define portInputRegister(port)    ( &(port->IN.reg) )
#define portModeRegister(port)     ( &(port->DIR.reg) )
#define digitalPinHasPWM(P)        ( g_APinDescription[P].ulPWMChannel != NOT_ON_PWM || g_APinDescription[P].ulTCChannel != NOT_ON_TIMER )

/*
 * digitalPinToTimer(..) is AVR-specific and is not defined for SAMD
 * architecture. If you need to check if a pin supports PWM you must
 * use digitalPinHasPWM(..).
 *
 * https://github.com/arduino/Arduino/issues/1833
 */
// #define digitalPinToTimer(P)

#if defined(__SAMD20J18__)
#define PIN_LED_13           (25u)
#define PIN_LED              PIN_LED_13
#define LED_BUILTIN          PIN_LED_13
#endif 

/*
 * Analog pins
 */
#if defined(__SAMD20E18__)
#define PIN_A0               (13ul)
#define PIN_A1               (14ul)
#define PIN_A2               (15ul)
#define PIN_A3               (16ul)
#define PIN_A4               (17ul)
#define PIN_A5               (18ul)
#define PIN_DAC0             (25ul)
#elif defined(__SAMD20J18__)
#define PIN_A0               (14ul)
#define PIN_A1               (15ul)
#define PIN_A2               (16ul)
#define PIN_A3               (17ul)
#define PIN_A4               (18ul)
#define PIN_A5               (19ul)
#define PIN_DAC0             (18ul)
#endif

static const uint8_t A0  = PIN_A0;
static const uint8_t A1  = PIN_A1;
static const uint8_t A2  = PIN_A2;
static const uint8_t A3  = PIN_A3;
static const uint8_t A4  = PIN_A4;
static const uint8_t A5  = PIN_A5;
static const uint8_t DAC0 = PIN_DAC0;
#define ADC_RESOLUTION		12

/* Other pins - No ATN
#define PIN_ATN              (38ul)
static const uint8_t ATN = PIN_ATN;*/

/*
 * Serial interfaces
 */
#if defined(__SAMD20E18__)
// Serial
#define PIN_SERIAL_RX       (0ul)
#define PIN_SERIAL_TX       (1ul)
#define PAD_SERIAL_TX       (UART_TX_PAD_2)
#define PAD_SERIAL_RX       (SERCOM_RX_PAD_3)
#elif defined(__SAMD20J18__)
// Serial (EDBG)
#define PIN_SERIAL_RX       (26ul)
#define PIN_SERIAL_TX       (27ul)
#define PAD_SERIAL_TX       (UART_TX_PAD_2)
#define PAD_SERIAL_RX       (SERCOM_RX_PAD_3)
// Serial1
#define PIN_SERIAL1_RX       (0ul)
#define PIN_SERIAL1_TX       (1ul)
#define PAD_SERIAL1_TX       (UART_TX_PAD_0)
#define PAD_SERIAL1_RX       (SERCOM_RX_PAD_1)
#endif

/*
 * SPI Interfaces
 */
#if defined(__SAMD20E18__)
#define SPI_INTERFACES_COUNT 1

#define PIN_SPI_MISO         (21ul)
#define PIN_SPI_MOSI         (22ul)
#define PIN_SPI_SCK          (23ul)
#define PERIPH_SPI           sercom1
#define PAD_SPI_TX           SPI_PAD_0_SCK_1
#define PAD_SPI_RX           SERCOM_RX_PAD_3

static const uint8_t SS	  = (7ul);	// SERCOM4 last PAD is present on A2 but HW SS isn't used. Set here only for reference.
static const uint8_t MOSI = PIN_SPI_MOSI ;
static const uint8_t MISO = PIN_SPI_MISO ;
static const uint8_t SCK  = PIN_SPI_SCK ;
#elif defined(__SAMD20J18__)
#define SPI_INTERFACES_COUNT 1

#define PIN_SPI_MISO         (22u)
#define PIN_SPI_MOSI         (23u)
#define PIN_SPI_SCK          (24u)
#define PERIPH_SPI           sercom0
#define PAD_SPI_TX           SPI_PAD_2_SCK_3
#define PAD_SPI_RX           SERCOM_RX_PAD_0

static const uint8_t SS	  = PIN_A2 ;	// SERCOM4 last PAD is present on A2 but HW SS isn't used. Set here only for reference.
static const uint8_t MOSI = PIN_SPI_MOSI ;
static const uint8_t MISO = PIN_SPI_MISO ;
static const uint8_t SCK  = PIN_SPI_SCK ;
#endif 

/*
 * Wire Interfaces
 */
#if defined(__SAMD20E18__)
#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA         (19ul)
#define PIN_WIRE_SCL         (20ul)
#define PERIPH_WIRE          sercom0
#define WIRE_IT_HANDLER      SERCOM0_Handler

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;
#elif defined(__SAMD20J18__)
#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA         (20ul)
#define PIN_WIRE_SCL         (21ul)
#define PERIPH_WIRE          sercom2
#define WIRE_IT_HANDLER      SERCOM2_Handler

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;
#endif

/*
 * Timer Counter Interfaces
 */
#if (defined(__SAMD20E18__) || defined(__SAMD20J18__))
#define TC_INTERFACES_COUNT 2

#define TC_INTERFACE_0 timerCounter3
#define TC_INTERFACE_0_HANDLER TC3_Handler

#define TC_INTERFACE_1 timerCounter4
#define TC_INTERFACE_4_HANDLER TC4_Handler
#endif // __SAMD20E18__ || __SAMD20J18__

#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus

/*	=========================
 *	===== SERCOM DEFINITION
 *	=========================
*/
#if (defined(__SAMD20E18__) || defined(__SAMD20J18__))
extern SERCOM sercom0;
extern SERCOM sercom1;
extern SERCOM sercom2;
extern SERCOM sercom3;
#if defined(__SAMD20J18__)
extern SERCOM sercom4;
extern SERCOM sercom5;
#endif // __SAMD20J18__
#endif // __SAMD20E18__ || __SAMD20J18__

extern TimerCounter Timer;
extern TimerCounter Timer1;
extern Uart Serial;
#if defined(__SAMD20J18__)
extern Uart Serial1;
#endif // __SAMD20J18__

#endif

// These serial port names are intended to allow libraries and architecture-neutral
// sketches to automatically default to the correct port name for a particular type
// of use.  For example, a GPS module would normally connect to SERIAL_PORT_HARDWARE_OPEN,
// the first hardware serial port whose RX/TX pins are not dedicated to another use.
//
// SERIAL_PORT_MONITOR        Port which normally prints to the Arduino Serial Monitor
//
// SERIAL_PORT_USBVIRTUAL     Port which is USB virtual serial
//
// SERIAL_PORT_LINUXBRIDGE    Port which connects to a Linux system via Bridge library
//
// SERIAL_PORT_HARDWARE       Hardware serial port, physical RX & TX pins.
//
// SERIAL_PORT_HARDWARE_OPEN  Hardware serial ports which are open for use.  Their RX & TX
//                            pins are NOT connected to anything by default.
// #define SERIAL_PORT_USBVIRTUAL      SerialUSB
#define SERIAL_PORT_MONITOR         Serial
// Serial has no physical pins broken out, so it's not listed as HARDWARE port
#if defined(__SAMD20J18__)
#define SERIAL_PORT_HARDWARE        Serial1
#define SERIAL_PORT_HARDWARE_OPEN   Serial1
#endif

#ifdef SAMD20
uint32_t __disableGlobalISR();
void __enableGlobalISR( uint32_t ISER );
#endif // SAMD20


#endif /* _VARIANT_ARDUINO_E_ */

