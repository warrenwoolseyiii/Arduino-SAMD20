/*
 * SPI Master library for Arduino Zero.
 * Copyright (c) 2015 Arduino LLC
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "SPI.h"

SPIClass::SPIClass( SERCOM *p_sercom, uint8_t uc_pinMISO, uint8_t uc_pinSCK,
                    uint8_t uc_pinMOSI, SercomSpiTXPad PadTx,
                    SercomRXPad PadRx )
{
    _busConfigured = false;
    if( p_sercom == NULL ) while ( 1 );
    _p_sercom = p_sercom;

    // pins
    _uc_pinMiso = uc_pinMISO;
    _uc_pinSCK = uc_pinSCK;
    _uc_pinMosi = uc_pinMOSI;

    // SERCOM pads
    _padTx = PadTx;
    _padRx = PadRx;

    // Default setup
    _clock = 4000000;
    _bitOrder = MSBFIRST;
    _dataMode = SPI_MODE0;
    _settingsInternal = SPISettings( _clock, _bitOrder, _dataMode );

    // Interrupts
    _interruptMode = spi_can_be_interrupted;

    // System clock setting
    _oldSystemClock = SystemCoreClock;
}

void SPIClass::begin()
{
    SPISettings s = SPISettings( _clock, _bitOrder, _dataMode );
    if( !( s == _settingsInternal ) || !_busConfigured ||
        ( _oldSystemClock != SystemCoreClock ) ) {
        _oldSystemClock = SystemCoreClock;
        _settingsInternal = s;
        config( _settingsInternal );
    }
}

void SPIClass::config( SPISettings settings )
{
    // PIO init
    pinMode( _uc_pinMiso, gArduinoPins[_uc_pinMiso].spi );
    pinMode( _uc_pinSCK, gArduinoPins[_uc_pinSCK].spi );
    pinMode( _uc_pinMosi, gArduinoPins[_uc_pinMosi].spi );

    _p_sercom->initSPI( _padTx, _padRx, SPI_CHAR_SIZE_8_BITS,
                        settings.bitOrder );
    _p_sercom->initSPIClock( settings.dataMode, settings.clockFreq );

    _p_sercom->enableSPI();

    _busConfigured = true;
}

void SPIClass::end()
{
    if( _busConfigured ) {
        _p_sercom->resetSPI();
        _p_sercom->endSPI();
        _busConfigured = false;
    }

    pinMode( _uc_pinMiso, OUTPUT );
    pinMode( _uc_pinSCK, OUTPUT );
    pinMode( _uc_pinMosi, OUTPUT );
    digitalWrite( _uc_pinMiso, HIGH );
    digitalWrite( _uc_pinSCK, HIGH );
    digitalWrite( _uc_pinMosi, HIGH );
}

void SPIClass::interruptMode( SPIInterruptMode_t intMode )
{
    ATOMIC_OPERATION( { _interruptMode = intMode; } )
}

void SPIClass::beginTransaction( SPISettings settings )
{
    if( _interruptMode == spi_blocking_transactions ) startAtomicOperation();

    if( !( settings == _settingsInternal ) || !_busConfigured ||
        ( _oldSystemClock != SystemCoreClock ) ) {
        _oldSystemClock = SystemCoreClock;
        _settingsInternal = settings;
        config( _settingsInternal );
    }
}

void SPIClass::endTransaction( void )
{
    if( _interruptMode == spi_blocking_transactions ) endAtomicOperation();
}

void SPIClass::setBitOrder( BitOrder order )
{
    _bitOrder = order;
}

void SPIClass::setDataMode( uint8_t mode )
{
    _dataMode = mode;
}

void SPIClass::setClockDivider( uint8_t div )
{
    _clock = SystemCoreClock / div;
}

byte SPIClass::transfer( uint8_t data )
{
    return _p_sercom->transferDataSPI( data );
}

uint16_t SPIClass::transfer16( uint16_t data )
{
    union
    {
        uint16_t val;
        struct
        {
            uint8_t lsb;
            uint8_t msb;
        };
    } t;

    t.val = data;

    if( _p_sercom->getDataOrderSPI() == LSB_FIRST ) {
        t.lsb = transfer( t.lsb );
        t.msb = transfer( t.msb );
    }
    else {
        t.msb = transfer( t.msb );
        t.lsb = transfer( t.lsb );
    }

    return t.val;
}

void SPIClass::transfer( void *buf, size_t count )
{
    uint8_t *buffer = reinterpret_cast<uint8_t *>( buf );
    for( size_t i = 0; i < count; i++ ) {
        *buffer = transfer( *buffer );
        buffer++;
    }
}

void SPIClass::attachInterrupt()
{
    // Should be enableInterrupt()
}

void SPIClass::detachInterrupt()
{
    // Should be disableInterrupt()
}

#if SPI_INTERFACES_COUNT > 0
/* In case new variant doesn't define these macros,
 * we put here the ones for Arduino Zero.
 *
 * These values should be different on some variants!
 *
 * The SPI PAD values can be found in cores/arduino/SERCOM.h:
 *   - SercomSpiTXPad
 *   - SercomRXPad
 */
#ifndef PERIPH_SPI
#define PERIPH_SPI sercom4
#define PAD_SPI_TX SPI_PAD_2_SCK_3
#define PAD_SPI_RX SERCOM_RX_PAD_0
#endif // PERIPH_SPI
SPIClass SPI( &PERIPH_SPI, PIN_SPI_MISO, PIN_SPI_SCK, PIN_SPI_MOSI, PAD_SPI_TX,
              PAD_SPI_RX );
#endif
#if SPI_INTERFACES_COUNT > 1
SPIClass SPI1( &PERIPH_SPI1, PIN_SPI1_MISO, PIN_SPI1_SCK, PIN_SPI1_MOSI,
               PAD_SPI1_TX, PAD_SPI1_RX );
#endif
#if SPI_INTERFACES_COUNT > 2
SPIClass SPI2( &PERIPH_SPI2, PIN_SPI2_MISO, PIN_SPI2_SCK, PIN_SPI2_MOSI,
               PAD_SPI2_TX, PAD_SPI2_RX );
#endif
#if SPI_INTERFACES_COUNT > 3
SPIClass SPI3( &PERIPH_SPI3, PIN_SPI3_MISO, PIN_SPI3_SCK, PIN_SPI3_MOSI,
               PAD_SPI3_TX, PAD_SPI3_RX );
#endif
#if SPI_INTERFACES_COUNT > 4
SPIClass SPI4( &PERIPH_SPI4, PIN_SPI4_MISO, PIN_SPI4_SCK, PIN_SPI4_MOSI,
               PAD_SPI4_TX, PAD_SPI4_RX );
#endif
#if SPI_INTERFACES_COUNT > 5
SPIClass SPI5( &PERIPH_SPI5, PIN_SPI5_MISO, PIN_SPI5_SCK, PIN_SPI5_MOSI,
               PAD_SPI5_TX, PAD_SPI5_RX );
#endif
