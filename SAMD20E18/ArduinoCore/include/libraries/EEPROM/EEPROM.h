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
    EEEPROM()
    {
      _flashMem = NVMFlash();
      
      // Calculate the various parameters for the EEEPROM space
      _minFlashPageSize = _flashMem.minimumEraseSize;
      _useableMemSize = _flashMem.EEEPROMSize;
      _numUsableBanks = _useableMemSize / _minFlashPageSize;
      _EEEPROMSize = ( _numUsableBanks / 2 ) * ( _minFlashPageSize - 1 );
      _flashEEEPROMStartAddr = _flashMem.startAddr;
      
      _bankStatus = (uint8_t *)malloc( _numUsableBanks );
      _bankUpToDate = false;
    }

    uint16_t getSize()
    {
      return _EEEPROMSize;
    }

    void write( uint16_t addr, void *data, uint16_t size )
    {
      
    }

    void read( uint16_t addr, void *data, uint16_t size )
    {
      uint32_t flashAddr = getFlashAddr( addr );
      uint16_t remainingBankSize = 
        ( _minFlashPageSize - 1 ) - ( addr % ( _minFlashPageSize - 1 ) );
      uint16_t readSize = ( remainingBankSize < size ? remainingBankSize : size );
      _flashMem.read( ( void * )flashAddr, data, readSize );
      if( remainingBankSize < size )
        read( addr + readSize, data + readSize, size - readSize );
    }

    void erase( uint16_t addr, uint16_t size )
    {

    }
  private:
    NVMFlash _flashMem;
    uint16_t _minFlashPageSize;
    uint16_t _useableMemSize;
    uint16_t _EEEPROMSize;
    uint8_t _numUsableBanks; 
    uint32_t _flashEEEPROMStartAddr;
    uint8_t *_bankStatus;
    bool _bankUpToDate;

    void retrieveBankStatus()
    {
      for( uint8_t i = 0; i < _numUsableBanks; i++ ) {
        _flashMem.read( ( void * )( _flashEEEPROMStartAddr + ( i * _minFlashPageSize ) ),
          &_bankStatus[i], 1 );
      }

      _bankUpToDate = true;
    }

    uint32_t getFlashAddr( uint16_t eeepromAddr )
    {
      uint8_t bankId = eeepromAddr / ( _minFlashPageSize - 1 );
      uint32_t addr = _flashEEEPROMStartAddr;
      
      // Index through the bank status array, if we get a match use that bank otherwise
      // just use the last empty bank we find
      if( !_bankUpToDate ) retrieveBankStatus();
      for( uint8_t i = 0; i < _numUsableBanks; i++ ) {
        if( _bankStatus[i] == bankId || _bankStatus[i] == 0xFF ) {
          addr = _flashEEEPROMStartAddr + ( i * _minFlashPageSize ) + 1;
          if( _bankStatus[i] == bankId )
            break;
        }
      }

      return addr;
    }
};
