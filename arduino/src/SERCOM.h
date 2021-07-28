/*
  Copyright (c) 2014 Arduino.  All right reserved.

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

#ifndef _SERCOM_CLASS_
#define _SERCOM_CLASS_

#include "sam.h"

#define SERCOM_NVIC_PRIORITY ( ( 1 << __NVIC_PRIO_BITS ) - 1 )

typedef enum
{
    UART_EXT_CLOCK = 0,
    UART_INT_CLOCK = 0x1u
} SercomUartMode;

typedef enum
{
    SPI_SLAVE_OPERATION = 0x2u,
    SPI_MASTER_OPERATION = 0x3u
} SercomSpiMode;

typedef enum
{
    I2C_SLAVE_OPERATION = 0x4u,
    I2C_MASTER_OPERATION = 0x5u
} SercomI2CMode;

typedef enum
{
    SERCOM_EVEN_PARITY = 0,
    SERCOM_ODD_PARITY,
    SERCOM_NO_PARITY
} SercomParityMode;

typedef enum
{
    SERCOM_STOP_BIT_1 = 0,
    SERCOM_STOP_BITS_2
} SercomNumberStopBit;

typedef enum
{
    MSB_FIRST = 0,
    LSB_FIRST
} SercomDataOrder;

typedef enum
{
    UART_CHAR_SIZE_8_BITS = 0,
    UART_CHAR_SIZE_9_BITS,
    UART_CHAR_SIZE_5_BITS = 0x5u,
    UART_CHAR_SIZE_6_BITS,
    UART_CHAR_SIZE_7_BITS
} SercomUartCharSize;

typedef enum
{
    SERCOM_RX_PAD_0 = 0,
    SERCOM_RX_PAD_1,
    SERCOM_RX_PAD_2,
    SERCOM_RX_PAD_3
} SercomRXPad;

typedef enum
{
    UART_TX_PAD_0 = 0x0ul, // Only for UART
    UART_TX_PAD_2 = 0x1ul, // Only for UART
    UART_TX_RTS_CTS_PAD_0_2_3 =
        0x2ul, // Only for UART with TX on PAD0, RTS on PAD2 and CTS on PAD3
} SercomUartTXPad;

typedef enum
{
    SAMPLE_RATE_x16 = 0x1, // Fractional
    SAMPLE_RATE_x8 = 0x3,  // Fractional
} SercomUartSampleRate;

typedef enum
{
    SERCOM_SPI_MODE_0 = 0, // CPOL : 0  | CPHA : 0
    SERCOM_SPI_MODE_1,     // CPOL : 0  | CPHA : 1
    SERCOM_SPI_MODE_2,     // CPOL : 1  | CPHA : 0
    SERCOM_SPI_MODE_3      // CPOL : 1  | CPHA : 1
} SercomSpiClockMode;

typedef enum
{
    SPI_PAD_0_SCK_1 = 0,
    SPI_PAD_2_SCK_3,
    SPI_PAD_3_SCK_1,
    SPI_PAD_0_SCK_3
} SercomSpiTXPad;

typedef enum
{
    SPI_CHAR_SIZE_8_BITS = 0x0ul,
    SPI_CHAR_SIZE_9_BITS
} SercomSpiCharSize;

typedef enum
{
    WIRE_UNKNOWN_STATE = 0x0ul,
    WIRE_IDLE_STATE,
    WIRE_OWNER_STATE,
    WIRE_BUSY_STATE
} SercomWireBusState;

typedef enum
{
    WIRE_MASTER_ACK_NO_ACTION = 0,
    WIRE_MASTER_ACK_REPEAT_START = 1,
    WIRE_MASTER_ACK_READ = 2,
    WIRE_MASTER_ACK_STOP = 3
} SercomMasterCommandWire;

typedef enum
{
    MODE_WIRE = 0,
    MODE_UART = 1,
    MODE_SPI = 2,
    MODE_NONE = 3
} SercomMode;

#define I2CM_ERR_NONE 0
#define I2CM_ERR_BUS_BUSY -1
#define I2CM_ERR_CONDITION -2
#define I2CM_ERR_LOW_TIMEOUT -3
#define I2CM_ERR_RX_NACK -4
#define I2CM_ERR_GENERAL -5

class SERCOM
{
  public:
    SERCOM( Sercom *s );

    bool sercomIRQEN();

    /* ========== UART ========== */
    void initUART( SercomUartMode mode, uint32_t baudrate = 0 );
    void initFrame( SercomUartCharSize charSize, SercomDataOrder dataOrder,
                    SercomParityMode    parityMode,
                    SercomNumberStopBit nbStopBits );
    void initPads( SercomUartTXPad txPad, SercomRXPad rxPad );

    void    resetUART( void );
    void    endUART( void );
    void    enableUART( void );
    void    flushUART( void );
    void    clearStatusUART( void );
    bool    availableDataUART( void );
    bool    isBufferOverflowErrorUART( void );
    bool    isFrameErrorUART( void );
    void    clearFrameErrorUART( void );
    bool    isParityErrorUART( void );
    bool    isDataRegisterEmptyUART( void );
    uint8_t readDataUART( void );
    int     writeDataUART( uint8_t data );
    bool    isUARTError();
    void    acknowledgeUARTError();
    void    enableDataRegisterEmptyInterruptUART();
    void    disableDataRegisterEmptyInterruptUART();

    /* ========== SPI ========== */
    void initSPI( SercomSpiTXPad mosi, SercomRXPad miso,
                  SercomSpiCharSize charSize, SercomDataOrder dataOrder );
    void initSPIClock( SercomSpiClockMode clockMode, uint32_t baudrate );

    void            resetSPI( void );
    void            endSPI( void );
    void            enableSPI( void );
    void            disableSPI( void );
    void            setDataOrderSPI( SercomDataOrder dataOrder );
    SercomDataOrder getDataOrderSPI( void );
    void            setBaudrateSPI( uint8_t divider );
    void            setClockModeSPI( SercomSpiClockMode clockMode );
    void            transferDataSPI( uint8_t *data, int len );
    uint8_t         transferDataSPI( uint8_t data );
    void            fastWriteDataSPI( uint8_t *data, int len );

    bool isBufferOverflowErrorSPI( void );
    bool isDataRegisterEmptySPI( void );
    bool isTransmitCompleteSPI( void );
    bool isReceiveCompleteSPI( void );

    /* ========== WIRE ========== */
    void resetWIRE( void );
    void enableWIRE( void );
    void disableWIRE( void );
    void endWire( void );
    void initMasterWIRE( bool fastMode );
    int  startTransmissionWIRE( uint8_t addr, bool isWrite );
    int  parseMasterWireStatus();
    int  waitForMBWire();
    int  waitForSBWire();
    void masterACKWire( bool ack );
    void prepareMasterCommandWIRE( uint8_t cmd );
    int  getStatusWIRE();
    void writeStatusWire( int status );
    int  sendDataMasterWIRE( uint8_t *data, int len, bool stop = false );
    int  readDataMasterWire( uint8_t *data, int len, bool ack, bool stop );

  private:
    Sercom *   sercom;
    SercomMode _mode;
    uint8_t    calculateBaudrateSynchronous( uint32_t baudrate );
    uint32_t   division( uint32_t dividend, uint32_t divisor );
    void       enableSERCOM();
    void       disableSERCOM();
    void       takeDownMode();
};

#endif
