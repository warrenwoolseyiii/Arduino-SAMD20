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
#include <stdint.h>
#include <Arduino.h>

class NVMFlash
{
  public:
    uint32_t minimumEraseSize;
    uint32_t EEEPROMSize;
    uint32_t startAddr;

    NVMFlash() 
    {
      NVMParams_t params = getNVMParams();
      minimumEraseSize = params.rowSize;
      EEEPROMSize = params.eepromSize;
      startAddr = params.nvmTotalSize - EEEPROMSize;
    }

    void read( const volatile void *flash_ptr, void *data, uint32_t size )
    {
      readFlash( flash_ptr, data, size );
    }

    void erase( uint32_t addr )
    {
      eraseRow( addr );
    }

    void write( const volatile void *flash_ptr, const void *data, uint32_t size )
    {
      writeFlash( flash_ptr, data, size );
    }
};

template <class T>
class EEEPROM
{
  public:
    EEEPROM() {};
  private:
    T _flashMem;
};

template <>
class EEEPROM <NVMFlash>
{
  public:
    EEEPROM();
    uint16_t getSize();
    void write( uint16_t addr, void *data, uint16_t size );
    void read( uint16_t addr, void *data, uint16_t size );
    void erase( uint16_t addr, uint16_t size );
  private:
    NVMFlash _flashMem;
    uint16_t _minFlashPageSize;
    uint16_t _useableMemSize;
    uint16_t _EEEPROMSize;
    uint8_t _numUsableBanks; 
    uint8_t _nextBankUp;
    uint32_t _flashEEEPROMStartAddr;
    uint8_t *_bankStatus;
    bool _bankUpToDate;

    void retrieveBankStatus();
    uint32_t findNextEmptyBank();
    uint32_t getFlashAddr( uint16_t eeepromAddr );
};
