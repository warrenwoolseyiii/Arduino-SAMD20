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

#include "SERCOM.h"
#include "variant.h"
#include <Arduino.h>

#ifndef WIRE_RISE_TIME_NANOSECONDS
// Default rise time in nanoseconds, based on 4.7K ohm pull up resistors
// you can override this value in your variant if needed
#define WIRE_RISE_TIME_NANOSECONDS 125
#endif

#define UART_SYNC_BUSY ( sercom->USART.STATUS.bit.SYNCBUSY )
#define UART_WAIT_SYNC while( UART_SYNC_BUSY )

#define SPI_SYNC_BUSY ( sercom->SPI.STATUS.bit.SYNCBUSY )
#define SPI_WAIT_SYNC while( SPI_SYNC_BUSY )

#define I2CM_SYNC_BUSY ( sercom->I2CM.STATUS.bit.SYNCBUSY )
#define I2CM_WAIT_SYNC while( I2CM_SYNC_BUSY )

#define I2CS_SYNC_BUSY ( sercom->I2CS.STATUS.bit.SYNCBUSY )
#define I2CS_WAIT_SYNC while( I2CS_SYNC_BUSY )

SERCOM::SERCOM( Sercom *s )
{
    sercom = s;
    _mode = MODE_NONE;
}

bool SERCOM::sercomIRQEN()
{
    IRQn_Type irqn = SERCOM0_IRQn;
    if( sercom == SERCOM1 ) {
        irqn = SERCOM1_IRQn;
    }
    if( sercom == SERCOM2 ) {
        irqn = SERCOM2_IRQn;
    }
    if( sercom == SERCOM3 ) {
        irqn = SERCOM3_IRQn;
    }
#if defined( SERCOM4 )
    if( sercom == SERCOM4 ) {
        irqn = SERCOM4_IRQn;
    }
#endif /* SERCOM4 */
#if defined( SERCOM5 )
    if( sercom == SERCOM5 ) {
        irqn = SERCOM5_IRQn;
    }
#endif /* SERCOM5 */

    return ( NVIC_GetEnableIRQ( irqn ) != 0 );
}

/* 	=========================
 *	===== Sercom UART
 *	=========================
 */
void SERCOM::initUART( SercomUartMode mode, uint32_t baudrate )
{
    if( _mode < MODE_NONE ) takeDownMode();
    _mode = MODE_UART;
    enableSERCOM();
    resetUART();

    // Setting the CTRLA register
    sercom->USART.CTRLA.reg = SERCOM_USART_CTRLA_MODE( mode );

    // Enable the receive data interrupt
    sercom->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;

    if( mode == UART_INT_CLOCK ) {
        uint64_t ratio = 1048576;
        ratio *= baudrate;
        ratio /= SystemCoreClock;
        sercom->USART.BAUD.reg = ( uint16_t )( 65536 - ratio );
    }
}

void SERCOM::initFrame( SercomUartCharSize charSize, SercomDataOrder dataOrder,
                        SercomParityMode    parityMode,
                        SercomNumberStopBit nbStopBits )
{
    // Setting the CTRLA register
    sercom->USART.CTRLA.reg |=
        SERCOM_USART_CTRLA_FORM( ( parityMode == SERCOM_NO_PARITY ? 0 : 1 ) ) |
        dataOrder << SERCOM_USART_CTRLA_DORD_Pos;

    // Setting the CTRLB register
    ATOMIC_OPERATION( {
        if( UART_SYNC_BUSY ) UART_WAIT_SYNC;
        sercom->USART.CTRLB.reg |=
            SERCOM_USART_CTRLB_CHSIZE( charSize ) |
            nbStopBits << SERCOM_USART_CTRLB_SBMODE_Pos |
            ( parityMode == SERCOM_NO_PARITY ? 0 : parityMode )
                << SERCOM_USART_CTRLB_PMODE_Pos; // If no parity use default
                                                 // value
    } )
}

void SERCOM::initPads( SercomUartTXPad txPad, SercomRXPad rxPad )
{
    sercom->USART.CTRLA.reg |=
        SERCOM_USART_CTRLA_TXPO | SERCOM_USART_CTRLA_RXPO( rxPad );

    // Enable Transceiver
    ATOMIC_OPERATION( {
        if( UART_SYNC_BUSY ) UART_WAIT_SYNC;
        sercom->USART.CTRLB.reg |=
            SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_RXEN;
    } )
}

void SERCOM::resetUART()
{
    ATOMIC_OPERATION( {
        if( UART_SYNC_BUSY ) UART_WAIT_SYNC;
        sercom->USART.CTRLA.bit.SWRST = 1;
    } )
    while( sercom->USART.CTRLA.bit.SWRST )
        ;
}

void SERCOM::endUART()
{
    disableSERCOM();
}

void SERCOM::enableUART()
{
    ATOMIC_OPERATION( {
        if( UART_SYNC_BUSY ) UART_WAIT_SYNC;
        sercom->USART.CTRLA.bit.ENABLE = 0x1u;
    } )
}

void SERCOM::flushUART()
{
    // Skip checking transmission completion if data register is empty
    if( isDataRegisterEmptyUART() ) return;

    // Wait for transmission to complete
    while( !sercom->USART.INTFLAG.bit.TXC )
        ;
}

void SERCOM::clearStatusUART()
{
    // Reset (with 0) the STATUS register
    sercom->USART.STATUS.reg = SERCOM_USART_STATUS_RESETVALUE;
}

bool SERCOM::availableDataUART()
{
    // RXC : Receive Complete
    return sercom->USART.INTFLAG.bit.RXC;
}

bool SERCOM::isUARTError()
{
    bool rtn = sercom->USART.STATUS.reg &
               ( SERCOM_USART_STATUS_BUFOVF | SERCOM_USART_STATUS_FERR |
                 SERCOM_USART_STATUS_PERR );
    return rtn;
}

void SERCOM::acknowledgeUARTError()
{
    if( isBufferOverflowErrorUART() )
        sercom->USART.STATUS.reg |= SERCOM_USART_STATUS_BUFOVF;
    if( isFrameErrorUART() )
        sercom->USART.STATUS.reg |= SERCOM_USART_STATUS_FERR;
    if( isParityErrorUART() )
        sercom->USART.STATUS.reg |= SERCOM_USART_STATUS_PERR;
}

bool SERCOM::isBufferOverflowErrorUART()
{
    // BUFOVF : Buffer Overflow
    return sercom->USART.STATUS.bit.BUFOVF;
}

bool SERCOM::isFrameErrorUART()
{
    // FERR : Frame Error
    return sercom->USART.STATUS.bit.FERR;
}

void SERCOM::clearFrameErrorUART()
{
    // clear FERR bit writing 1 status bit
    sercom->USART.STATUS.bit.FERR = 1;
}

bool SERCOM::isParityErrorUART()
{
    // PERR : Parity Error
    return sercom->USART.STATUS.bit.PERR;
}

bool SERCOM::isDataRegisterEmptyUART()
{
    // DRE : Data Register Empty
    return sercom->USART.INTFLAG.bit.DRE;
}

uint8_t SERCOM::readDataUART()
{
    return sercom->USART.DATA.bit.DATA;
}

int SERCOM::writeDataUART( uint8_t data )
{
    sercom->USART.DATA.reg = (uint16_t)data;
    return 1;
}

void SERCOM::enableDataRegisterEmptyInterruptUART()
{
    sercom->USART.INTENSET.reg = SERCOM_USART_INTENSET_DRE;
}

void SERCOM::disableDataRegisterEmptyInterruptUART()
{
    sercom->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
}

/*	=========================
 *	===== Sercom SPI
 *	=========================
 */
void SERCOM::initSPI( SercomSpiTXPad mosi, SercomRXPad miso,
                      SercomSpiCharSize charSize, SercomDataOrder dataOrder )
{
    if( _mode < MODE_NONE ) takeDownMode();
    _mode = MODE_SPI;
    enableSERCOM();
    resetSPI();

    // Setting the CTRLA register
    sercom->SPI.CTRLA.reg =
        SERCOM_SPI_CTRLA_MODE_SPI_MASTER | SERCOM_SPI_CTRLA_DOPO( mosi ) |
        SERCOM_SPI_CTRLA_DIPO( miso ) | dataOrder << SERCOM_SPI_CTRLA_DORD_Pos;

    // Setting the CTRLB register
    ATOMIC_OPERATION( {
        if( SPI_SYNC_BUSY ) SPI_WAIT_SYNC;
        sercom->SPI.CTRLB.reg =
            SERCOM_SPI_CTRLB_CHSIZE( charSize ) |
            SERCOM_SPI_CTRLB_RXEN; // Active the SPI receiver.
    } )
}

void SERCOM::initSPIClock( SercomSpiClockMode clockMode, uint32_t baudrate )
{
    // Extract data from clockMode
    int cpha, cpol;

    if( ( clockMode & ( 0x1ul ) ) == 0 )
        cpha = 0;
    else
        cpha = 1;

    if( ( clockMode & ( 0x2ul ) ) == 0 )
        cpol = 0;
    else
        cpol = 1;

    // Setting the CTRLA register
    sercom->SPI.CTRLA.reg |= ( cpha << SERCOM_SPI_CTRLA_CPHA_Pos ) |
                             ( cpol << SERCOM_SPI_CTRLA_CPOL_Pos );

    // Synchronous arithmetic
    sercom->SPI.BAUD.reg = calculateBaudrateSynchronous( baudrate );
}

void SERCOM::resetSPI()
{
    // Setting the Software Reset bit to 1
    ATOMIC_OPERATION( {
        if( SPI_SYNC_BUSY ) SPI_WAIT_SYNC;
        sercom->SPI.CTRLA.bit.SWRST = 1;
    } )
    while( sercom->SPI.CTRLA.bit.SWRST )
        ;
}

void SERCOM::endSPI()
{
    disableSERCOM();
}

void SERCOM::enableSPI()
{
    ATOMIC_OPERATION( {
        if( SPI_SYNC_BUSY ) SPI_WAIT_SYNC;
        sercom->SPI.CTRLA.bit.ENABLE = 1;
    } )
}

void SERCOM::disableSPI()
{
    ATOMIC_OPERATION( {
        if( SPI_SYNC_BUSY ) SPI_WAIT_SYNC;
        sercom->SPI.CTRLA.bit.ENABLE = 0;
    } )
}

void SERCOM::setDataOrderSPI( SercomDataOrder dataOrder )
{
    // Register enable-protected
    disableSPI();
    sercom->SPI.CTRLA.bit.DORD = dataOrder;
    enableSPI();
}

SercomDataOrder SERCOM::getDataOrderSPI()
{
    return ( sercom->SPI.CTRLA.bit.DORD ? LSB_FIRST : MSB_FIRST );
}

void SERCOM::setBaudrateSPI( uint8_t divider )
{
    // Can't divide by 0
    if( divider == 0 ) return;

    // Register enable-protected
    disableSPI();
    sercom->SPI.BAUD.reg =
        calculateBaudrateSynchronous( SystemCoreClock / divider );
    enableSPI();
}

void SERCOM::setClockModeSPI( SercomSpiClockMode clockMode )
{
    int cpha, cpol;
    if( ( clockMode & ( 0x1ul ) ) == 0 )
        cpha = 0;
    else
        cpha = 1;

    if( ( clockMode & ( 0x2ul ) ) == 0 )
        cpol = 0;
    else
        cpol = 1;

    // Register enable-protected
    disableSPI();
    sercom->SPI.CTRLA.bit.CPOL = cpol;
    sercom->SPI.CTRLA.bit.CPHA = cpha;
    enableSPI();
}

void SERCOM::fastWriteDataSPI( uint8_t *data, int len )
{
    Sercom * ser = sercom;
    uint32_t ctrlb = ser->SPI.CTRLB.reg;
    ser->SPI.CTRLB.bit.RXEN = 0;
    volatile uint8_t *pDATA = (uint8_t *)&ser->SPI.DATA.reg;
    volatile uint8_t *pflag = &ser->SPI.INTFLAG.reg;
    uint8_t *         end = data + len;
    while( data < end ) {
        uint8_t d = *data++;
        asm volatile( "l1: ldrb r2, [%2]\n\t"
                      "lsl r2, r2, #31\n\t"
                      "bpl l1\n\t"
                      "strb %0, [%1]\n\t"
                      :
                      : "r"( d ), "r"( pDATA ), "r"( pflag )
                      : "cc", "r2" );
    }
    while( !ser->SPI.INTFLAG.bit.TXC )
        ;
    ser->SPI.CTRLB.reg = ctrlb;
}

void SERCOM::transferDataSPI( uint8_t *data, int len )
{
    if( !len ) return;
    volatile uint8_t *pDATA = (uint8_t *)&sercom->SPI.DATA.reg;
    uint8_t *         end = data + len;
    Sercom *          ser = sercom;
    while( !ser->SPI.INTFLAG.bit.DRE )
        ;
    *pDATA = *data;
    if( --len ) {
        while( !ser->SPI.INTFLAG.bit.DRE )
            ;
        *pDATA = data[1];
        uint8_t d;
        if( data < end - 2 ) {
            d = data[2];
            while( data < end - 3 ) {
                if( ser->SPI.INTFLAG.bit.DRE ) {
                    *data++ = *pDATA;
                    *pDATA = d;
                    d = data[2];
                }
            }
            while( !ser->SPI.INTFLAG.bit.DRE )
                ;
            *data++ = *pDATA;
            *pDATA = d;
        }
    }
    while( data < end ) {
        if( ser->SPI.INTFLAG.bit.RXC ) *data++ = *pDATA;
    }
}

uint8_t SERCOM::transferDataSPI( uint8_t data )
{
    // Write data and wait for return data
    sercom->SPI.DATA.bit.DATA = data;
    while( sercom->SPI.INTFLAG.bit.RXC == 0 )
        ;

    return sercom->SPI.DATA.bit.DATA;
}

bool SERCOM::isBufferOverflowErrorSPI()
{
    return sercom->SPI.STATUS.bit.BUFOVF;
}

bool SERCOM::isDataRegisterEmptySPI()
{
    return sercom->SPI.INTFLAG.bit.DRE;
}

uint8_t SERCOM::calculateBaudrateSynchronous( uint32_t baudrate )
{
    if( baudrate >= SystemCoreClock ) baudrate = ( SystemCoreClock / 2 );
    return SystemCoreClock / ( 2 * baudrate ) - 1;
}

/*	=========================
 *	===== Sercom WIRE
 *	=========================
 */
void SERCOM::resetWIRE()
{
    // I2CM OR I2CS, no matter SWRST is the same bit.
    // Setting the Software bit to 1
    ATOMIC_OPERATION( {
        if( I2CM_SYNC_BUSY ) I2CM_WAIT_SYNC;
        sercom->I2CM.CTRLA.bit.SWRST = 1;
    } )
    while( sercom->I2CM.CTRLA.bit.SWRST )
        ;
}

void SERCOM::enableWIRE()
{
    // I2C Master and Slave modes share the ENABLE bit function.
    // Enable the I2C master mode
    ATOMIC_OPERATION( {
        if( I2CM_SYNC_BUSY ) I2CM_WAIT_SYNC;
        sercom->I2CM.CTRLA.bit.ENABLE = 1;
    } )

    // Setting bus idle mode
    ATOMIC_OPERATION( {
        if( I2CM_SYNC_BUSY ) I2CM_WAIT_SYNC;
        sercom->I2CM.STATUS.bit.BUSSTATE = 1;
    } )
}

void SERCOM::disableWIRE()
{
    // I2C Master and Slave modes share the ENABLE bit function.
    // Enable the I2C master mode
    ATOMIC_OPERATION( {
        if( I2CM_SYNC_BUSY ) I2CM_WAIT_SYNC;
        sercom->I2CM.CTRLA.bit.ENABLE = 0;
    } )
}

void SERCOM::endWire()
{
    resetWIRE();
    disableSERCOM();
}

void SERCOM::initMasterWIRE( bool fastMode )
{
    if( _mode < MODE_NONE ) takeDownMode();
    _mode = MODE_WIRE;

    enableSERCOM();
    resetWIRE();

    // Use SCL low timeout, standard 2 wire master mode
    sercom->I2CM.CTRLA.reg =
        SERCOM_I2CM_CTRLA_MODE( SERCOM_I2CM_CTRLA_MODE_I2C_MASTER_Val ) |
        SERCOM_I2CM_CTRLA_LOWTOUT | SERCOM_I2CM_CTRLA_INACTOUT( 1 );

    // Enable smart mode
    sercom->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;

    // Baud rate constants
    const float i2c_fast = 0.00000125;
    const float i2c_slow = 0.000005;
    const float trise_2 = 0.000000125;

    // Set the baud rate
    uint8_t baud =
        ( uint8_t )( SystemCoreClock * ( fastMode ? i2c_fast : i2c_slow ) -
                     SystemCoreClock * trise_2 - 4 );
    sercom->I2CM.BAUD.bit.BAUD = baud;

    enableWIRE();
}

int SERCOM::startTransmissionWIRE( uint8_t addr, bool isWrite )
{
    // 7-bits address + 1-bits R/W
    addr <<= 0x01;
    if( !isWrite ) addr |= 0x01;

    // Don't try anything if the bus is busy
    int status = parseMasterWireStatus();
    if( status == I2CM_ERR_BUS_BUSY || status == I2CM_ERR_CONDITION )
        return status;

    // Send start and address
    ATOMIC_OPERATION( {
        if( I2CM_SYNC_BUSY ) I2CM_WAIT_SYNC;
        sercom->I2CM.ADDR.bit.ADDR = addr;
    } )

    return I2CM_ERR_NONE;
}

int SERCOM::parseMasterWireStatus()
{
    // Check in descending order according to figure 26-5 in the data sheet
    int status = getStatusWIRE();

    // Bus busy
    if( ( status & SERCOM_I2CM_STATUS_BUSSTATE( 3 ) ) ==
        SERCOM_I2CM_STATUS_BUSSTATE( 3 ) ) {
        return I2CM_ERR_BUS_BUSY;
    }

    // Error condition
    if( status & ( SERCOM_I2CM_STATUS_BUSERR | SERCOM_I2CM_STATUS_ARBLOST ) )
        return I2CM_ERR_CONDITION;

    // Low time out
    if( status & SERCOM_I2CM_STATUS_LOWTOUT ) return I2CM_ERR_LOW_TIMEOUT;

    // RX NACK
    if( status & SERCOM_I2CM_STATUS_RXNACK ) return I2CM_ERR_RX_NACK;

    return I2CM_ERR_NONE;
}

int SERCOM::waitForMBWire()
{
    // Wait for MB to be set
    while( !sercom->I2CM.INTFLAG.bit.MB )
        ;
    return parseMasterWireStatus();
}

int SERCOM::waitForSBWire()
{
    // Wait for SB to be set
    while( !sercom->I2CM.INTFLAG.bit.SB ) {
        // If the slave NACKS the address, the MB bit will be set
        if( sercom->I2CM.INTFLAG.bit.MB ) parseMasterWireStatus();
    }
    return I2CM_ERR_NONE;
}

void SERCOM::masterACKWire( bool ack )
{
    sercom->I2CM.CTRLB.bit.ACKACT = ( ack ? 0 : 1 );
}

void SERCOM::prepareMasterCommandWIRE( uint8_t cmd )
{
    ATOMIC_OPERATION( {
        if( I2CM_SYNC_BUSY ) I2CM_WAIT_SYNC;
        sercom->I2CM.CTRLB.bit.CMD = cmd;
    } )
}

int SERCOM::getStatusWIRE()
{
    return sercom->I2CM.STATUS.reg;
}

void SERCOM::writeStatusWire( int status )
{
    ATOMIC_OPERATION( {
        if( I2CM_SYNC_BUSY ) I2CM_WAIT_SYNC;
        sercom->I2CM.STATUS.reg = ( status & 0xFFFF );
    } )
}

int SERCOM::sendDataMasterWIRE( uint8_t *data, int len, bool stop )
{
    // Wait for the address bits
    int status = waitForMBWire();
    if( status != I2CM_ERR_NONE ) return status;

    for( int i = 0; i < len; i++ ) {
        // Send data, wait for it to come out the SR
        sercom->I2CM.DATA.bit.DATA = data[i];
        status = waitForMBWire();
        if( status != I2CM_ERR_NONE ) return status;

        // Send a stop
        if( stop && ( i == ( len - 1 ) ) )
            prepareMasterCommandWIRE( WIRE_MASTER_ACK_STOP );
    }

    return parseMasterWireStatus();
}

int SERCOM::readDataMasterWire( uint8_t *data, int len, bool ack, bool stop )
{
    // Send an ACK
    masterACKWire( ack );

    for( int i = 0; i < len; i++ ) {
        // Wait for the bits to clear
        if( waitForSBWire() != I2CM_ERR_NONE ) break;

        // Send a stop
        if( stop && ( i == ( len - 1 ) ) ) {
            masterACKWire( false );
            prepareMasterCommandWIRE( WIRE_MASTER_ACK_STOP );
        }

        data[i] = sercom->I2CM.DATA.bit.DATA;
    }

    return parseMasterWireStatus();
}

void SERCOM::enableSERCOM()
{
    uint32_t id = GCLK_CLKCTRL_ID_SERCOM0_CORE_Val;
    uint32_t apbMask = PM_APBCMASK_SERCOM0;
    uint32_t irqn = SERCOM0_IRQn;
    if( sercom == SERCOM1 ) {
        id = GCLK_CLKCTRL_ID_SERCOM1_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM1;
        irqn = SERCOM1_IRQn;
    }
    if( sercom == SERCOM2 ) {
        id = GCLK_CLKCTRL_ID_SERCOM2_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM2;
        irqn = SERCOM2_IRQn;
    }
    if( sercom == SERCOM3 ) {
        id = GCLK_CLKCTRL_ID_SERCOM3_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM3;
        irqn = SERCOM3_IRQn;
    }
#if defined( SERCOM4 )
    if( sercom == SERCOM4 ) {
        id = GCLK_CLKCTRL_ID_SERCOM4_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM4;
        irqn = SERCOM4_IRQn;
    }
#endif /* SERCOM4 */
#if defined( SERCOM5 )
    if( sercom == SERCOM5 ) {
        id = GCLK_CLKCTRL_ID_SERCOM5_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM5;
        irqn = SERCOM5_IRQn;
    }
#endif /* SERCOM5 */

    // Ensure that PORT is enabled
    enableAPBBClk( PM_APBBMASK_PORT, 1 );

    initGenericClk( GCLK_CLKCTRL_GEN_GCLK0_Val, id );
    enableAPBCClk( apbMask, 1 );
    NVIC_EnableIRQ( (IRQn_Type)irqn );
}

void SERCOM::disableSERCOM()
{
    uint32_t id = GCLK_CLKCTRL_ID_SERCOM0_CORE_Val;
    uint32_t apbMask = PM_APBCMASK_SERCOM0;
    uint32_t irqn = SERCOM0_IRQn;
    if( sercom == SERCOM1 ) {
        id = GCLK_CLKCTRL_ID_SERCOM1_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM1;
        irqn = SERCOM1_IRQn;
    }
    if( sercom == SERCOM2 ) {
        id = GCLK_CLKCTRL_ID_SERCOM2_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM2;
        irqn = SERCOM2_IRQn;
    }
    if( sercom == SERCOM3 ) {
        id = GCLK_CLKCTRL_ID_SERCOM3_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM3;
        irqn = SERCOM3_IRQn;
    }
#if defined( SERCOM4 )
    if( sercom == SERCOM4 ) {
        id = GCLK_CLKCTRL_ID_SERCOM4_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM4;
        irqn = SERCOM4_IRQn;
    }
#endif /* SERCOM4 */
#if defined( SERCOM5 )
    if( sercom == SERCOM5 ) {
        id = GCLK_CLKCTRL_ID_SERCOM5_CORE_Val;
        apbMask = PM_APBCMASK_SERCOM5;
        irqn = SERCOM5_IRQn;
    }
#endif /* SERCOM5 */

    NVIC_DisableIRQ( (IRQn_Type)irqn );
    enableAPBCClk( apbMask, 0 );
    disableGenericClk( id );
}

void SERCOM::takeDownMode()
{
    switch( _mode ) {
        case MODE_WIRE: endWire(); break;
        case MODE_UART: endUART(); break;
        case MODE_SPI: endSPI(); break;
        case MODE_NONE:
        default: break; _mode = MODE_NONE;
    }
}
