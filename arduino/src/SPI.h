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

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <Arduino.h>

// SPI_HAS_TRANSACTION means SPI has
//   - beginTransaction()
//   - endTransaction()
//   - usingInterrupt()
//   - SPISetting(clock, bitOrder, dataMode)
#define SPI_HAS_TRANSACTION 1

// SPI_HAS_NOTUSINGINTERRUPT means that SPI has notUsingInterrupt() method
#define SPI_HAS_NOTUSINGINTERRUPT 1

#define SPI_MODE0 0x02
#define SPI_MODE1 0x00
#define SPI_MODE2 0x03
#define SPI_MODE3 0x01

typedef enum
{
    spi_can_be_interrupted = 0,
    spi_blocking_transactions = 1,
    spi_external_pin_interrupt = 2,
} SPIInterruptMode_t;

class SPISettings
{
  public:
    SPISettings( uint32_t clock, BitOrder bitOrder, uint8_t dataMode )
    {
        if( __builtin_constant_p( clock ) ) {
            init_AlwaysInline( clock, bitOrder, dataMode );
        }
        else {
            init_MightInline( clock, bitOrder, dataMode );
        }
    }

    // Default speed set to 4MHz, SPI mode set to MODE 0 and Bit order set to
    // MSB first.
    SPISettings()
    {
        init_AlwaysInline( 4000000, MSBFIRST, SPI_MODE0 );
    }

    void init_MightInline( uint32_t clock, BitOrder bitOrder, uint8_t dataMode )
    {
        init_AlwaysInline( clock, bitOrder, dataMode );
    }

    void init_AlwaysInline( uint32_t clock, BitOrder bitOrder,
                            uint8_t dataMode )
        __attribute__( ( __always_inline__ ) )
    {
        this->clockFreq = clock;

        this->bitOrder = ( bitOrder == MSBFIRST ? MSB_FIRST : LSB_FIRST );

        switch( dataMode ) {
            case SPI_MODE0: this->dataMode = SERCOM_SPI_MODE_0; break;
            case SPI_MODE1: this->dataMode = SERCOM_SPI_MODE_1; break;
            case SPI_MODE2: this->dataMode = SERCOM_SPI_MODE_2; break;
            case SPI_MODE3: this->dataMode = SERCOM_SPI_MODE_3; break;
            default: this->dataMode = SERCOM_SPI_MODE_0; break;
        }
    }

    bool operator==( const SPISettings &x )
    {
        return ( ( this->clockFreq == x.clockFreq ) &&
                 ( this->dataMode == x.dataMode ) &&
                 ( this->bitOrder == x.bitOrder ) );
    }

    SPISettings &operator=( const SPISettings &x )
    {
        this->clockFreq = x.clockFreq;
        this->bitOrder = x.bitOrder;
        this->dataMode = x.dataMode;
        return *this;
    }

  private:
    uint32_t           clockFreq;
    SercomSpiClockMode dataMode;
    SercomDataOrder    bitOrder;

    friend class SPIClass;
};

class SPIClass
{
  public:
    SPIClass( SERCOM *p_sercom, uint8_t uc_pinMISO, uint8_t uc_pinSCK,
              uint8_t uc_pinMOSI, SercomSpiTXPad, SercomRXPad );

    byte     transfer( uint8_t data );
    uint16_t transfer16( uint16_t data );
    void     transfer( void *buf, size_t count );
    void     fastSend( const void *buf, size_t count );

    // Transaction Functions
    void interruptMode( SPIInterruptMode_t intMode );
    void beginTransaction( SPISettings settings );
    void beginTransaction();
    void endTransaction( void );

    // SPI Configuration methods
    void attachInterrupt();
    void detachInterrupt();

    void begin();
    void config( SPISettings settings );
    void end();

    void setBitOrder( BitOrder order );
    void setDataMode( uint8_t uc_mode );
    void setClockDivider( uint8_t uc_div );

  private:
    SERCOM *_p_sercom;
    uint8_t _uc_pinMiso;
    uint8_t _uc_pinMosi;
    uint8_t _uc_pinSCK;

    SercomSpiTXPad _padTx;
    SercomRXPad    _padRx;

    bool               _busConfigured;
    SPIInterruptMode_t _interruptMode;

    // Legacy setup support
    SPISettings _settingsInternal;
    uint32_t    _clock;
    BitOrder    _bitOrder;
    uint8_t     _dataMode;

    // Internal clock setting
    uint32_t _oldSystemClock;
};

#if SPI_INTERFACES_COUNT > 0
extern SPIClass SPI;
#endif
#if SPI_INTERFACES_COUNT > 1
extern SPIClass SPI1;
#endif
#if SPI_INTERFACES_COUNT > 2
extern SPIClass SPI2;
#endif
#if SPI_INTERFACES_COUNT > 3
extern SPIClass SPI3;
#endif
#if SPI_INTERFACES_COUNT > 4
extern SPIClass SPI4;
#endif
#if SPI_INTERFACES_COUNT > 5
extern SPIClass SPI5;
#endif

// For compatibility with sketches designed for AVR @ 16 MHz
// New programs should use SPI.beginTransaction to set the SPI clock
#define SPI_CLOCK_DIV2 2
#define SPI_CLOCK_DIV4 4
#define SPI_CLOCK_DIV8 8
#define SPI_CLOCK_DIV16 16
#define SPI_CLOCK_DIV32 32
#define SPI_CLOCK_DIV64 64
#define SPI_CLOCK_DIV128 128

#endif /* _SPI_H_INCLUDED */
