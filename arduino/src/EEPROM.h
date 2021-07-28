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
#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define EEEPROM_RAM_CACHE_SIZE 2048
#define EEEPROM_NUM_FLASH_BANKS 4

#define EEEPROM_KEY_LEN 2
#define EEEPROM_KEY_LOC ( EEEPROM_CHECK_SUM_LOC - EEEPROM_KEY_LEN )
#define EEEPROM_KEY 0xEA72
#define EEEPROM_CHECK_SUM_LEN 4
#define EEEPROM_CHECK_SUM_LOC ( EEEPROM_RAM_CACHE_SIZE - EEEPROM_CHECK_SUM_LEN )

#define EEEPROM_ERR_OK 0
#define EEEPROM_ERR_INVALID_CACHE_ADDR -1
#define EEEPROM_ERR_NULL_PTR -2
#define EEEPROM_ERR_FLASH_NOT_INIT -3
#define EEEPROM_ERR_FLASH_READ_FAIL -4
#define EEEPROM_ERR_FLASH_WRITE_FAIL -5
#define EEEPROM_ERR_FLASH_ERASE_FAIL -6
#define EEEPROM_ERR_CANT_FIND_BANK -7

typedef struct
{
    int eeepromBaseFlashAddress, pageSize;
    int ( *flashWrite )( int addr, uint8_t *data, int len );
    int ( *flashRead )( int addr, uint8_t *data, int len );
    int ( *flashErase )( int addr, int len );
    int ( *flashInvalidate )( int addr, int len );
} FlashMemoryInterface_t;

class EEEPROM
{
  public:
    EEEPROM();
    EEEPROM( FlashMemoryInterface_t *f );
    int      begin();
    uint16_t getSize();
    int      setFlashMemoryInterface( FlashMemoryInterface_t *f );
    bool     hasChange();

    int commit();
    int clearFlash();
    int write( int addr, void *data, int size );
    int write( int addr, uint8_t value )
    {
        return write( addr, &value, 1 );
    }

    int     read( int addr, void *data, int size );
    uint8_t read( int addr )
    {
        uint8_t byte;
        read( addr, &byte, 1 );
        return byte;
    }

    int  erase( int addr, int size );
    void end();

  private:
    FlashMemoryInterface_t _flashInterface;

    bool    _flashInterfaceInit, _hasChange;
    uint8_t _eepromRAMCache[EEEPROM_RAM_CACHE_SIZE];
    int     _flashPageNdx;

    uint32_t crc32();

    int validateCacheAddr( int addr );
    int _write( int addr, uint8_t *data, int size );
    int _read( int addr, uint8_t *data, int size );
    int _erase( int addr, int size );
};

#endif /* EEPROM_H_ */
