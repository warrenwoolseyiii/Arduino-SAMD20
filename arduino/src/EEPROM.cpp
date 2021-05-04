/*
  Written by Warren Woolsey

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

#include "EEPROM.h"
#include "atomic.h"

EEEPROM::EEEPROM()
{
    _flashInterfaceInit = false;
    _hasChange = false;
    _flashPageNdx = 0;
    memset( _eepromRAMCache, 0xFF, EEEPROM_RAM_CACHE_SIZE );
}

EEEPROM::EEEPROM( FlashMemoryInterface_t *f )
{
    _flashPageNdx = 0;
    _hasChange = false;
    _flashInterfaceInit = ( setFlashMemoryInterface( f ) == EEEPROM_ERR_OK );
    memset( _eepromRAMCache, 0xFF, EEEPROM_RAM_CACHE_SIZE );
}

int EEEPROM::begin()
{
    if( !_flashInterfaceInit ) return EEEPROM_ERR_FLASH_NOT_INIT;

    // Load the cache from flash memory until we get the active page
    for( _flashPageNdx = 0; _flashPageNdx < EEEPROM_NUM_FLASH_BANKS;
         _flashPageNdx++ ) {

        // Make the address and read from flash memory
        int addr = _flashInterface.eeepromBaseFlashAddress +
                   ( _flashPageNdx * EEEPROM_RAM_CACHE_SIZE );
        if( _flashInterface.flashRead( addr, _eepromRAMCache,
                                       EEEPROM_RAM_CACHE_SIZE ) !=
            EEEPROM_RAM_CACHE_SIZE ) {
            return EEEPROM_ERR_FLASH_READ_FAIL;
        }

        // Check for the key
        uint16_t key = ( _eepromRAMCache[EEEPROM_KEY_LOC] << 8 ) |
                       _eepromRAMCache[EEEPROM_KEY_LOC + 1];
        if( key == EEEPROM_KEY ) {

            // If the key is good, look at the checksum
            uint32_t crcRead =
                ( _eepromRAMCache[EEEPROM_CHECK_SUM_LOC] << 24 ) |
                ( _eepromRAMCache[EEEPROM_CHECK_SUM_LOC + 1] << 16 ) |
                ( _eepromRAMCache[EEEPROM_CHECK_SUM_LOC + 2] << 8 ) |
                _eepromRAMCache[EEEPROM_CHECK_SUM_LOC + 3];

            if( crcRead == crc32() ) return EEEPROM_ERR_OK;
        }
    }

    return EEEPROM_ERR_CANT_FIND_BANK;
}

uint16_t EEEPROM::getSize()
{
    return EEEPROM_RAM_CACHE_SIZE - ( EEEPROM_KEY_LEN + EEEPROM_CHECK_SUM_LEN );
}

int EEEPROM::setFlashMemoryInterface( FlashMemoryInterface_t *f )
{
    if( f == NULL ) return EEEPROM_ERR_NULL_PTR;
    memcpy( &_flashInterface, f, sizeof( FlashMemoryInterface_t ) );
    _flashInterfaceInit = true;
    return EEEPROM_ERR_OK;
}

bool EEEPROM::hasChange()
{
    return _hasChange;
}

int EEEPROM::commit()
{
    // Make the next address
    int prevNdx = _flashPageNdx;
    _flashPageNdx = ( _flashPageNdx + 1 ) % EEEPROM_NUM_FLASH_BANKS;
    int addr = _flashInterface.eeepromBaseFlashAddress +
               ( _flashPageNdx * EEEPROM_RAM_CACHE_SIZE );

    // Check if we need to erase the previous page when we are done
    bool erasePreviousPage = false;
    if( addr % _flashInterface.pageSize == 0 ) erasePreviousPage = true;

    // Make the new check sum, add the key, save to flash
    uint32_t crc = crc32();
    uint16_t key = EEEPROM_KEY;
    _eepromRAMCache[EEEPROM_KEY_LOC] = ( key >> 8 ) & 0xFF;
    _eepromRAMCache[EEEPROM_KEY_LOC + 1] = key & 0xFF;
    _eepromRAMCache[EEEPROM_CHECK_SUM_LOC] = ( crc >> 24 ) & 0xFF;
    _eepromRAMCache[EEEPROM_CHECK_SUM_LOC + 1] = ( crc >> 16 ) & 0xFF;
    _eepromRAMCache[EEEPROM_CHECK_SUM_LOC + 2] = ( crc >> 8 ) & 0xFF;
    _eepromRAMCache[EEEPROM_CHECK_SUM_LOC + 3] = crc & 0xFF;
    if( _flashInterface.flashWrite( addr, _eepromRAMCache,
                                    EEEPROM_RAM_CACHE_SIZE ) !=
        EEEPROM_RAM_CACHE_SIZE ) {
        return EEEPROM_ERR_FLASH_WRITE_FAIL;
    }

    // Erase if we need to
    if( erasePreviousPage ) {
        addr = _flashInterface.eeepromBaseFlashAddress +
               ( prevNdx * EEEPROM_RAM_CACHE_SIZE );
        if( _flashInterface.flashErase( addr, _flashInterface.pageSize ) !=
            _flashInterface.pageSize ) {
            return EEEPROM_ERR_FLASH_ERASE_FAIL;
        }
    }
    else {
        // Wipe the previous header
        addr = _flashInterface.eeepromBaseFlashAddress +
               ( prevNdx * EEEPROM_RAM_CACHE_SIZE );

        if( _flashInterface.flashInvalidate(
                addr + ( EEEPROM_RAM_CACHE_SIZE -
                         ( EEEPROM_CHECK_SUM_LEN + EEEPROM_KEY_LEN ) ),
                ( EEEPROM_CHECK_SUM_LEN + EEEPROM_KEY_LEN ) ) !=
            ( EEEPROM_CHECK_SUM_LEN + EEEPROM_KEY_LEN ) ) {
            return EEEPROM_ERR_FLASH_WRITE_FAIL;
        }
    }

    _hasChange = false;
    return EEEPROM_ERR_OK;
}

int EEEPROM::clearFlash()
{
    // Loop through all the banks and erase them
    for( int i = 0; i < EEEPROM_NUM_FLASH_BANKS; i++ ) {

        // Make a new address and erase the underlying page if we are on a new
        // page
        int addr = _flashInterface.eeepromBaseFlashAddress +
                   ( i * EEEPROM_RAM_CACHE_SIZE );
        if( ( addr % _flashInterface.pageSize ) == 0 ) {
            if( _flashInterface.flashErase( addr, _flashInterface.pageSize ) !=
                _flashInterface.pageSize ) {
                return EEEPROM_ERR_FLASH_ERASE_FAIL;
            }
        }
    }

    _flashPageNdx = 0;
    return EEEPROM_ERR_OK;
}

int EEEPROM::write( int addr, void *data, int size )
{
    // Wrap the write operation in a blocking statement so we don't get
    // interrupted mid write
    int code;
    ATOMIC_OPERATION( code = _write( addr, (uint8_t *)data, size ); )
    return code;
}

int EEEPROM::read( int addr, void *data, int size )
{
    // Wrap the read operation in a blocking statement so we don't get
    // interrupted mid read
    int code;
    ATOMIC_OPERATION( code = _read( addr, (uint8_t *)data, size ); )
    return code;
}

int EEEPROM::erase( int addr, int size )
{
    // Wrap the erase operation in a blocking statement so we don't get
    // interrupted mid erase
    int code;
    ATOMIC_OPERATION( code = _erase( addr, size ); )
    return code;
}

// CRC32 using the Castagnoli polynomial (same one as used by the Intel crc32
// instruction) CRC-32C (iSCSI) polynomial in reversed bit order.
#define POLY 0x82f63b78
uint32_t EEEPROM::crc32()
{
    int k;

    uint32_t crc = 0;
    for( int i = 0; i < EEEPROM_CHECK_SUM_LOC; i++ ) {
        crc ^= _eepromRAMCache[i];
        for( k = 0; k < 8; k++ ) crc = crc & 1 ? ( crc >> 1 ) ^ POLY : crc >> 1;
    }
    return ~crc;
}

int EEEPROM::validateCacheAddr( int addr )
{
    if( addr > ( EEEPROM_RAM_CACHE_SIZE -
                 ( EEEPROM_CHECK_SUM_LEN + EEEPROM_KEY_LEN ) ) ||
        EEEPROM_RAM_CACHE_SIZE < 0 ) {
        return EEEPROM_ERR_INVALID_CACHE_ADDR;
    }
    return EEEPROM_ERR_OK;
}

int EEEPROM::_write( int addr, uint8_t *data, int size )
{
    // Always check the cache address!
    if( validateCacheAddr( addr + size ) != EEEPROM_ERR_OK )
        return EEEPROM_ERR_INVALID_CACHE_ADDR;

    // NULL check
    if( data == NULL ) return EEEPROM_ERR_NULL_PTR;

    //  Cache write
    for( int i = 0; i < size; i++ ) _eepromRAMCache[addr + i] = data[i];
    _hasChange = true;
    return size;
}

int EEEPROM::_read( int addr, uint8_t *data, int size )
{
    // Always check the cache address!
    if( validateCacheAddr( addr + size ) != EEEPROM_ERR_OK )
        return EEEPROM_ERR_INVALID_CACHE_ADDR;

    // NULL check
    if( data == NULL ) return EEEPROM_ERR_NULL_PTR;

    //  Cache read
    for( int i = 0; i < size; i++ ) data[i] = _eepromRAMCache[addr + i];
    return size;
}

int EEEPROM::_erase( int addr, int size )
{
    // Always check the cache address!
    if( validateCacheAddr( addr ) != EEEPROM_ERR_OK )
        return EEEPROM_ERR_INVALID_CACHE_ADDR;

    //  Cache write
    for( int i = 0; i < size; i++ ) _eepromRAMCache[addr + i] = 0xFF;
    _hasChange = true;
    return size;
}

void EEEPROM::end()
{}
