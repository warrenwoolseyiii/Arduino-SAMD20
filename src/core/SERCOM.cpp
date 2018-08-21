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

SERCOM::SERCOM( Sercom *s )
{
    sercom = s;
    _mode = MODE_NONE;
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
    sercom->USART.CTRLB.reg |=
        SERCOM_USART_CTRLB_CHSIZE( charSize ) |
        nbStopBits << SERCOM_USART_CTRLB_SBMODE_Pos |
        ( parityMode == SERCOM_NO_PARITY ? 0 : parityMode )
            << SERCOM_USART_CTRLB_PMODE_Pos; // If no parity use default value
}

void SERCOM::initPads( SercomUartTXPad txPad, SercomRXPad rxPad )
{
    sercom->USART.CTRLA.reg |=
        SERCOM_USART_CTRLA_TXPO | SERCOM_USART_CTRLA_RXPO( rxPad );

    // Enable Transceiver
    sercom->USART.CTRLB.reg |=
        SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_RXEN;
}

void SERCOM::resetUART()
{
    sercom->USART.CTRLA.bit.SWRST = 1;
    while( sercom->USART.CTRLA.bit.SWRST || sercom->USART.STATUS.bit.SYNCBUSY )
        ;
}

void SERCOM::endUART()
{
    disableSERCOM();
}

void SERCOM::enableUART()
{
    sercom->USART.CTRLA.bit.ENABLE = 0x1u;
    while( sercom->USART.STATUS.bit.SYNCBUSY )
        ;
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
    // Wait for data register to be empty
    while( !isDataRegisterEmptyUART() )
        ;

    // Put data into DATA register
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
    sercom->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_CHSIZE( charSize ) |
                            SERCOM_SPI_CTRLB_RXEN; // Active the SPI receiver.
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
    sercom->SPI.CTRLA.bit.SWRST = 1;
    while( sercom->SPI.CTRLA.bit.SWRST || sercom->SPI.STATUS.bit.SYNCBUSY )
        ;
}

void SERCOM::endSPI()
{
    disableSERCOM();
}

void SERCOM::enableSPI()
{
    sercom->SPI.CTRLA.bit.ENABLE = 1;
    while( sercom->SPI.STATUS.bit.SYNCBUSY )
        ;
}

void SERCOM::disableSPI()
{
    sercom->SPI.CTRLA.bit.ENABLE = 0;
    while( sercom->SPI.STATUS.bit.SYNCBUSY )
        ;
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
    sercom->I2CM.CTRLA.bit.SWRST = 1;
    while( sercom->I2CM.CTRLA.bit.SWRST || sercom->I2CM.STATUS.bit.SYNCBUSY )
        ;
}

void SERCOM::enableWIRE()
{
    // I2C Master and Slave modes share the ENABLE bit function.
    // Enable the I2C master mode
    sercom->I2CM.CTRLA.bit.ENABLE = 1;
    while( sercom->I2CM.STATUS.bit.SYNCBUSY )
        ;

    // Setting bus idle mode
    sercom->I2CM.STATUS.bit.BUSSTATE = 1;
    while( sercom->I2CM.STATUS.bit.SYNCBUSY )
        ;
}

void SERCOM::disableWIRE()
{
    // I2C Master and Slave modes share the ENABLE bit function.
    // Enable the I2C master mode
    sercom->I2CM.CTRLA.bit.ENABLE = 0;
    while( sercom->I2CM.STATUS.bit.SYNCBUSY )
        ;
}

void SERCOM::endWire()
{
    resetWIRE();
    disableSERCOM();
}

void SERCOM::initSlaveWIRE( uint8_t ucAddress, bool enableGeneralCall )
{
    if( _mode < MODE_NONE ) takeDownMode();
    _mode = MODE_WIRE;

    enableSERCOM();
    resetWIRE();

    // Set slave mode
    sercom->I2CS.CTRLA.bit.MODE = I2C_SLAVE_OPERATION;

    sercom->I2CS.ADDR.reg =
        SERCOM_I2CS_ADDR_ADDR( ucAddress &
                               0x7Ful ) |    // 0x7F, select only 7 bits
        SERCOM_I2CS_ADDR_ADDRMASK( 0x00ul ); // 0x00, only match exact address
    if( enableGeneralCall ) {
        sercom->I2CS.ADDR.reg |=
            SERCOM_I2CS_ADDR_GENCEN; // enable general call (address 0x00)
    }

    // Set the interrupt register
    sercom->I2CS.INTENSET.reg = SERCOM_I2CS_INTENSET_PREC |   // Stop
                                SERCOM_I2CS_INTENSET_AMATCH | // Address Match
                                SERCOM_I2CS_INTENSET_DRDY;    // Data Ready

    while( sercom->I2CM.STATUS.bit.SYNCBUSY )
        ;
}

void SERCOM::initMasterWIRE( uint32_t baudrate )
{
    if( _mode < MODE_NONE ) takeDownMode();
    _mode = MODE_WIRE;

    enableSERCOM();
    resetWIRE();

    // Set master mode and enable SCL Clock Stretch mode (stretch after ACK bit)
    sercom->I2CM.CTRLA.reg =
        SERCOM_I2CM_CTRLA_MODE( I2C_MASTER_OPERATION ) /* |
                            SERCOM_I2CM_CTRLA_SCLSM*/;

    // Enable Smart mode and Quick Command
    // sercom->I2CM.CTRLB.reg =  SERCOM_I2CM_CTRLB_SMEN /*|
    // SERCOM_I2CM_CTRLB_QCEN*/ ;

    // Enable all interrupts
    //  sercom->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB |
    //  SERCOM_I2CM_INTENSET_SB | SERCOM_I2CM_INTENSET_ERROR ;

    // Synchronous arithmetic baudrate
    sercom->I2CM.BAUD.bit.BAUD =
        SystemCoreClock / ( 2 * baudrate ) - 5 -
        ( ( ( SystemCoreClock / 1000000 ) * WIRE_RISE_TIME_NANOSECONDS ) /
          ( 2 * 1000 ) );
}

void SERCOM::prepareNackBitWIRE( void )
{
    if( isMasterWIRE() ) {
        // Send a NACK
        sercom->I2CM.CTRLB.bit.ACKACT = 1;
    }
    else {
        sercom->I2CS.CTRLB.bit.ACKACT = 1;
    }
}

void SERCOM::prepareAckBitWIRE( void )
{
    if( isMasterWIRE() ) {
        // Send an ACK
        sercom->I2CM.CTRLB.bit.ACKACT = 0;
    }
    else {
        sercom->I2CS.CTRLB.bit.ACKACT = 0;
    }
}

void SERCOM::prepareCommandBitsWire( uint8_t cmd )
{
    if( isMasterWIRE() ) {
        sercom->I2CM.CTRLB.bit.CMD = cmd;
        while( sercom->I2CM.STATUS.bit.SYNCBUSY )
            ;
    }
    else {
        sercom->I2CS.CTRLB.bit.CMD = cmd;
    }
}

bool SERCOM::startTransmissionWIRE( uint8_t                 address,
                                    SercomWireReadWriteFlag flag )
{
    // 7-bits address + 1-bits R/W
    address = ( address << 0x1ul ) | flag;

    // Wait idle or owner bus mode
    while( !isBusIdleWIRE() && !isBusOwnerWIRE() )
        ;

    // Send start and address
    sercom->I2CM.ADDR.bit.ADDR = address;

    // Address Transmitted
    if( flag == WIRE_WRITE_FLAG ) // Write mode
    {
        while( !sercom->I2CM.INTFLAG.bit.MB ) {
            // Wait transmission complete
        }
    }
    else // Read mode
    {
        while( !sercom->I2CM.INTFLAG.bit.SB ) {
            // If the slave NACKS the address, the MB bit will be set.
            // In that case, send a stop condition and return false.
            if( sercom->I2CM.INTFLAG.bit.MB ) {
                sercom->I2CM.CTRLB.bit.CMD = 3; // Stop condition
                return false;
            }
            // Wait transmission complete
        }

        // Clean the 'Slave on Bus' flag, for further usage.
        // sercom->I2CM.INTFLAG.bit.SB = 0x1ul;
    }

    // ACK received (0: ACK, 1: NACK)
    if( sercom->I2CM.STATUS.bit.RXNACK ) {
        return false;
    }
    else {
        return true;
    }
}

bool SERCOM::sendDataMasterWIRE( uint8_t data )
{
    // Send data
    sercom->I2CM.DATA.bit.DATA = data;

    // Wait transmission successful
    while( !sercom->I2CM.INTFLAG.bit.MB ) {

        // If a bus error occurs, the MB bit may never be set.
        // Check the bus error bit and bail if it's set.
        if( sercom->I2CM.STATUS.bit.BUSERR ) {
            return false;
        }
    }

    // Problems on line? nack received?
    if( sercom->I2CM.STATUS.bit.RXNACK )
        return false;
    else
        return true;
}

bool SERCOM::sendDataSlaveWIRE( uint8_t data )
{
    // Send data
    sercom->I2CS.DATA.bit.DATA = data;

    // Problems on line? nack received?
    if( !sercom->I2CS.INTFLAG.bit.DRDY || sercom->I2CS.STATUS.bit.RXNACK )
        return false;
    else
        return true;
}

bool SERCOM::isMasterWIRE( void )
{
    return sercom->I2CS.CTRLA.bit.MODE == I2C_MASTER_OPERATION;
}

bool SERCOM::isSlaveWIRE( void )
{
    return sercom->I2CS.CTRLA.bit.MODE == I2C_SLAVE_OPERATION;
}

bool SERCOM::isBusIdleWIRE( void )
{
    return sercom->I2CM.STATUS.bit.BUSSTATE == WIRE_IDLE_STATE;
}

bool SERCOM::isBusOwnerWIRE( void )
{
    return sercom->I2CM.STATUS.bit.BUSSTATE == WIRE_OWNER_STATE;
}

bool SERCOM::isDataReadyWIRE( void )
{
    return sercom->I2CS.INTFLAG.bit.DRDY;
}

bool SERCOM::isStopDetectedWIRE( void )
{
    return sercom->I2CS.INTFLAG.bit.PREC;
}

bool SERCOM::isRestartDetectedWIRE( void )
{
    return sercom->I2CS.STATUS.bit.SR;
}

bool SERCOM::isAddressMatch( void )
{
    return sercom->I2CS.INTFLAG.bit.AMATCH;
}

bool SERCOM::isMasterReadOperationWIRE( void )
{
    return sercom->I2CS.STATUS.bit.DIR;
}

bool SERCOM::isRXNackReceivedWIRE( void )
{
    return sercom->I2CM.STATUS.bit.RXNACK;
}

int SERCOM::availableWIRE( void )
{
    if( isMasterWIRE() )
        return sercom->I2CM.INTFLAG.bit.SB;
    else
        return sercom->I2CS.INTFLAG.bit.DRDY;
}

uint8_t SERCOM::readDataWIRE( void )
{
    if( isMasterWIRE() ) {
        while( sercom->I2CM.INTFLAG.bit.SB == 0 ) {
            // Waiting complete receive
        }

        return sercom->I2CM.DATA.bit.DATA;
    }
    else {
        return sercom->I2CS.DATA.reg;
    }
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
