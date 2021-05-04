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

#include "Uart.h"
#include "Arduino.h"

#define NO_RTS_PIN 255
#define NO_CTS_PIN 255
#define RTS_RX_THRESHOLD 10

Uart::Uart( SERCOM *_s, uint8_t _pinRX, uint8_t _pinTX, SercomRXPad _padRX,
            SercomUartTXPad _padTX )
    : Uart( _s, _pinRX, _pinTX, _padRX, _padTX, NO_RTS_PIN, NO_CTS_PIN )
{}

Uart::Uart( SERCOM *_s, uint8_t _pinRX, uint8_t _pinTX, SercomRXPad _padRX,
            SercomUartTXPad _padTX, uint8_t _pinRTS, uint8_t _pinCTS )
{
    sercom = _s;
    uc_pinRX = _pinRX;
    uc_pinTX = _pinTX;
    uc_padRX = _padRX;
    uc_padTX = _padTX;
    uc_pinRTS = _pinRTS;
    uc_pinCTS = _pinCTS;
    initialized = false;
}

void Uart::begin( unsigned long baudrate )
{
    begin( baudrate, SERIAL_8N1 );
}

void Uart::begin( unsigned long baudrate, uint16_t config )
{
    pinMode( uc_pinRX, gArduinoPins[uc_pinRX].uart );
    pinMode( uc_pinTX, gArduinoPins[uc_pinTX].uart );

    if( uc_padTX == UART_TX_RTS_CTS_PAD_0_2_3 ) {
        if( uc_pinCTS != NO_CTS_PIN ) {
            // pinPeripheral( uc_pinCTS, g_APinDescription[uc_pinCTS].ulPinType
            // );
        }
    }

    if( uc_pinRTS != NO_RTS_PIN ) {
        pinMode( uc_pinRTS, OUTPUT );

        pul_outsetRTS = &PORT->Group[gArduinoPins[uc_pinRTS].port].OUTSET.reg;
        pul_outclrRTS = &PORT->Group[gArduinoPins[uc_pinRTS].port].OUTCLR.reg;
        ul_pinMaskRTS = ( 1ul << gArduinoPins[uc_pinRTS].pin );

        *pul_outclrRTS = ul_pinMaskRTS;
    }

    sercom->initUART( UART_INT_CLOCK, baudrate );
    sercom->initFrame( extractCharSize( config ), LSB_FIRST,
                       extractParity( config ), extractNbStopBit( config ) );
    sercom->initPads( uc_padTX, uc_padRX );

    sercom->enableUART();
    initialized = true;
}

void Uart::end()
{
    if( initialized ) {
        sercom->resetUART();
        sercom->endUART();
    }

    pinMode( uc_pinRX, OUTPUT );
    pinMode( uc_pinTX, OUTPUT );
    digitalWrite( uc_pinRX, HIGH );
    digitalWrite( uc_pinTX, HIGH );

    _rxBuffer.Flush();
    _txBuffer.Flush();
}

void Uart::flush()
{
    if( _txBuffer.GetNumObjStored() ) {

        // If interrupts are not enabled then force the bytes out in a loop
        if( !sercom->sercomIRQEN() ) {
            while( _txBuffer.GetNumObjStored() ) {
                while( !sercom->isDataRegisterEmptyUART() )
                    ;
                uint8_t data = 0;
                _txBuffer.DeQueue( &data );
                sercom->writeDataUART( data );
            }
        }
        else {

            // Otherwise just sit here until everything gets flushed
            uint32_t n;
            do {
                startAtomicOperation();
                n = _txBuffer.GetNumObjStored();
                endAtomicOperation();
            } while( n );
        }
    }
}

void Uart::IrqHandler()
{
    // Handle frame error
    if( sercom->isFrameErrorUART() ) {
        sercom->readDataUART();
        sercom->clearFrameErrorUART();
    }

    // Read bytes
    if( sercom->availableDataUART() ) {
        _rxBuffer.Queue( sercom->readDataUART() );

        if( uc_pinRTS != NO_RTS_PIN ) {
            // RX buffer space is below the threshold, de-assert RTS
            if( _rxBuffer.GetAvailableSpace() < RTS_RX_THRESHOLD ) {
                *pul_outsetRTS = ul_pinMaskRTS;
            }
        }
    }

    // Send bytes
    if( sercom->isDataRegisterEmptyUART() ) {
        if( _txBuffer.GetNumObjStored() ) {
            uint8_t data;
            _txBuffer.DeQueue( &data );
            sercom->writeDataUART( data );
        }
        else {
            // Disable this interrupt if empty
            sercom->disableDataRegisterEmptyInterruptUART();
        }
    }

    if( sercom->isUARTError() ) {
        sercom->acknowledgeUARTError();
        // TODO: if (sercom->isBufferOverflowErrorUART()) ....
        // TODO: if (sercom->isParityErrorUART()) ....
        sercom->clearStatusUART();
    }
}

int Uart::available()
{
    return _rxBuffer.GetNumObjStored();
}

int Uart::availableForWrite()
{
    return _txBuffer.GetAvailableSpace();
}

int Uart::peek()
{
    uint8_t *data = _rxBuffer.AccessElement( 1 );
    int      rtn = -1;
    if( data != NULL ) rtn = *data;
    return rtn;
}

int Uart::read()
{
    int c = -1;
    startAtomicOperation();
    _rxBuffer.DeQueue( (uint8_t *)&c );
    endAtomicOperation();

    if( uc_pinRTS != NO_RTS_PIN ) {
        // If there is enough space in the RX buffer, assert RTS
        if( _rxBuffer.GetAvailableSpace() > RTS_RX_THRESHOLD ) {
            *pul_outclrRTS = ul_pinMaskRTS;
        }
    }

    return c;
}

size_t Uart::write( const uint8_t *data, size_t size )
{
    startAtomicOperation();
    int rtn = _txBuffer.Queue( (uint8_t *)data, size );
    endAtomicOperation();
    sercom->enableDataRegisterEmptyInterruptUART();
    return rtn;
}

size_t Uart::write( const uint8_t data )
{
    return write( &data, 1 );
}

SercomNumberStopBit Uart::extractNbStopBit( uint16_t config )
{
    switch( config & HARDSER_STOP_BIT_MASK ) {
        case HARDSER_STOP_BIT_1:
        default: return SERCOM_STOP_BIT_1;

        case HARDSER_STOP_BIT_2: return SERCOM_STOP_BITS_2;
    }
}

SercomUartCharSize Uart::extractCharSize( uint16_t config )
{
    switch( config & HARDSER_DATA_MASK ) {
        case HARDSER_DATA_5: return UART_CHAR_SIZE_5_BITS;

        case HARDSER_DATA_6: return UART_CHAR_SIZE_6_BITS;

        case HARDSER_DATA_7: return UART_CHAR_SIZE_7_BITS;

        case HARDSER_DATA_8:
        default: return UART_CHAR_SIZE_8_BITS;
    }
}

SercomParityMode Uart::extractParity( uint16_t config )
{
    switch( config & HARDSER_PARITY_MASK ) {
        case HARDSER_PARITY_NONE:
        default: return SERCOM_NO_PARITY;

        case HARDSER_PARITY_EVEN: return SERCOM_EVEN_PARITY;

        case HARDSER_PARITY_ODD: return SERCOM_ODD_PARITY;
    }
}
