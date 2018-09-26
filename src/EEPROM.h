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

class EEEPROM
{
  public:
    EEEPROM();
    void     begin();
    uint16_t getSize();
    void     write( uint16_t addr, uint8_t value )
    {
        write( addr, &value, 1 );
    }

    void    write( uint16_t addr, void *data, uint16_t size );
    void    read( uint16_t addr, void *data, uint16_t size );
    uint8_t read( uint16_t addr )
    {
        uint8_t byte;
        read( addr, &byte, 1 );
        return byte;
    }

    void erase( uint16_t addr, uint16_t size );

  private:
    uint16_t _minFlashPageSize, _effectivePageSize;
    uint16_t _useableMemSize, _EEEPROMSize;
    uint8_t  _numUsableBanks, _nextBankUp;
    uint8_t *_bankStatus;
    bool     _bankUpToDate;
    uint32_t _flashEEEPROMStartAddr;

    void     retrieveBankStatus();
    uint32_t getNextEmptyBankAddr();
    uint32_t getFlashAddr( uint16_t eeepromAddr );
};

#endif /* EEPROM_H_ */
