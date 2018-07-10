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

EEEPROM<NVMFlash>::EEEPROM()
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
  _nextBankUp = 0;
}

uint16_t EEEPROM<NVMFlash>::getSize()
{
  return _EEEPROMSize;
}

void EEEPROM<NVMFlash>::write( uint16_t addr, void *data, uint16_t size )
{
  if( addr + size > _EEEPROMSize ) return;
      
  // Cache the full page associated with the addr and set the bank ID
  uint8_t *cache = ( uint8_t * )malloc( _minFlashPageSize );
  if( cache == NULL ) return;
  read( ( addr - ( addr % ( _minFlashPageSize - 1 ) ) ),
    &cache[1], _minFlashPageSize - 1 );

  // Determine if the write will extend past the existing bank, if it does we will need to
  // recursively call write() until the write request has been fulfilled.
  uint16_t remainingBankSize =
    ( _minFlashPageSize - 1 ) - ( addr % ( _minFlashPageSize - 1 ) );
  uint16_t writeSize = ( remainingBankSize < size ? remainingBankSize : size );

  uint16_t cacheOffset = ( addr % ( _minFlashPageSize - 1 ) ) + 1;
  memcpy( &cache[cacheOffset], data, writeSize );
  cache[0] = addr / ( _minFlashPageSize - 1 );

  uint32_t flashWriteAddr, flashEraseAddr;
  flashWriteAddr = findNextEmptyBank();
  flashEraseAddr = getFlashAddr( addr );
  _flashMem.erase( flashEraseAddr );
  _flashMem.write( ( void * )flashWriteAddr, cache, _minFlashPageSize );

  free( cache );
  _bankUpToDate = false;
  if( remainingBankSize < size )
    write( addr + writeSize, ( uint8_t * )data + writeSize, size - writeSize );
}

void EEEPROM<NVMFlash>::read( uint16_t addr, void *data, uint16_t size )
{
  if( addr + size > _EEEPROMSize ) return;

  uint32_t flashAddr = getFlashAddr( addr );

  // Determine if the read will extend past the end of the flash bank, if
  // it does then we need to update the EEEPROM address and recursively call
  // read() until we have fulfilled the request.
  uint16_t remainingBankSize =
    ( _minFlashPageSize - 1 ) - ( addr % ( _minFlashPageSize - 1 ) );
  uint16_t readSize = ( remainingBankSize < size ? remainingBankSize : size );
  _flashMem.read( ( void * )flashAddr, data, readSize );
  if( remainingBankSize < size )
    read( addr + readSize, ( uint8_t * )data + readSize, size - readSize );
}

void EEEPROM<NVMFlash>::erase( uint16_t addr, uint16_t size )
{
  uint8_t *data = ( uint8_t * )malloc( size );
  if( data == NULL ) return;
  memset( data, 0xFF, size );
  write( addr, data, size );
  free( data );
}

void EEEPROM<NVMFlash>::retrieveBankStatus()
{
  for( uint8_t i = 0; i < _numUsableBanks; i++ ) {
    _flashMem.read( ( void * )( _flashEEEPROMStartAddr + ( i * _minFlashPageSize ) ),
      &_bankStatus[i], 1 );
  }

  _bankUpToDate = true;
}

uint32_t EEEPROM<NVMFlash>::findNextEmptyBank()
{
  uint32_t addr = _flashEEEPROMStartAddr;

  if( !_bankUpToDate ) retrieveBankStatus();
  while( _bankStatus[_nextBankUp] != 0xFF ) {
    if( ++_nextBankUp >= _numUsableBanks )
      _nextBankUp = 0;
  }

  addr += _nextBankUp * _minFlashPageSize;
  return addr;
}

uint32_t EEEPROM<NVMFlash>::getFlashAddr( uint16_t eeepromAddr )
{
  uint8_t bankId = eeepromAddr / ( _minFlashPageSize - 1 );
  uint32_t addr = _flashEEEPROMStartAddr;
      
  // Index through the bank status array, if we get a match use that bank otherwise
  // just use the last empty bank we find.
  if( !_bankUpToDate ) retrieveBankStatus();
  for( uint8_t i = 0; i < _numUsableBanks; i++ ) {
    if( _bankStatus[i] == bankId || _bankStatus[i] == 0xFF ) {
      addr = _flashEEEPROMStartAddr + ( i * _minFlashPageSize ) + 1
        + ( eeepromAddr % ( _minFlashPageSize - 1 ) );
      if( _bankStatus[i] == bankId )
        break;
    }
  }

  return addr;
}