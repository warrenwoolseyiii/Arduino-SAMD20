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

// Emulated EEPROM (EEEPROM) is a software layer of EEPROM that utilizes
// Flash memory as the underlying storage mechanism. This is NOT an EEPROM
// driver, it is instead an abstraction layer that lets the user treat a
// particular portion of Flash memory as EEPROM. Each Flash memory page is laid
// out in the following manner:
// Byte[0]                        -> BankID
// Bytes[1 - _minFlashPageSize]   -> Data
// Each Flash memory page is treated as a "bank", and a range of EEEPROM
// addressable space is associated with a bank. For example, if
// _minFlashPageSize == 256, than EEEPROM addresses 0 to 254 are located
// in bank 0. Banks are not consecutive in memory and are moved when writes
// occur with in the bank. This is done in an effort to reduce the write
// and erase load on each individual Flash memory page.

#include "EEPROM.h"
#include "NVM.h"

EEEPROM::EEEPROM()
{
    _EEEPROMSize = 0;
    _bankStatus = NULL;
}

void EEEPROM::begin()
{
    // Do not launch another session of emulated EEPROM, which will cause
    // _bankStatus to become corrupted if it has already been allocated
    if( _EEEPROMSize == 0 && _bankStatus == NULL ) {
        NVMParams_t params = getNVMParams();

        // Flash page characteristics
        _minFlashPageSize = params.rowSize;
        _effectivePageSize = _minFlashPageSize - 1;
        _flashEEEPROMStartAddr = params.nvmTotalSize - params.eepromSize;

        // Usable memory space
        _useableMemSize = params.eepromSize;
        _numUsableBanks = _useableMemSize / _minFlashPageSize;
        _EEEPROMSize = ( _numUsableBanks / 2 ) * _effectivePageSize;

        // Flash bank status indicators
        _bankStatus = (uint8_t *)malloc( _numUsableBanks );
        _bankUpToDate = false;
        _nextBankUp = 0;
    }
}

uint16_t EEEPROM::getSize()
{
    return _EEEPROMSize;
}

void EEEPROM::write( uint16_t addr, void *data, uint16_t size )
{
    if( addr + size > _EEEPROMSize ) return;

    // Cache the full page associated with the address, set the ID
    // in case the cached page is an empty bank.
    uint8_t *cache = (uint8_t *)malloc( _minFlashPageSize );
    if( cache == NULL ) return;
    read( ( addr - ( addr % _effectivePageSize ) ), &cache[1],
          _effectivePageSize );
    cache[0] = addr / _effectivePageSize;

    // Determine if the write will extend past the existing bank, if it does we
    // will need to recursively call write() until the write request has been
    // fulfilled.
    uint16_t remainingBankSize =
        _effectivePageSize - ( addr % _effectivePageSize );
    uint16_t writeSize =
        ( remainingBankSize < size ? remainingBankSize : size );

    // Update the cached page with the write data.
    uint16_t cacheOffset = ( addr % _effectivePageSize ) + 1;
    memcpy( &cache[cacheOffset], data, writeSize );

    // Write the updated cache to the next bank and erase the current bank.
    uint32_t flashWriteAddr, flashEraseAddr;
    flashWriteAddr = getNextEmptyBankAddr();
    flashEraseAddr = getFlashAddr( addr );
    eraseRow( flashEraseAddr );
    writeFlash( (void *)flashWriteAddr, cache, _minFlashPageSize );

    // Free the cache and continue the write if necessary.
    free( cache );
    _bankUpToDate = false;
    if( remainingBankSize < size )
        write( addr + writeSize, (uint8_t *)data + writeSize,
               size - writeSize );
}

void EEEPROM::read( uint16_t addr, void *data, uint16_t size )
{
    if( addr + size > _EEEPROMSize ) return;

    // Determine if the read will extend past the end of the flash bank, if
    // it does then we need to update the EEEPROM address and recursively call
    // read() until we have fulfilled the request.
    uint16_t remainingBankSize =
        _effectivePageSize - ( addr % _effectivePageSize );
    uint16_t readSize = ( remainingBankSize < size ? remainingBankSize : size );

    // Perform the read and continue if necessary
    uint32_t flashAddr = getFlashAddr( addr );
    readFlash( (void *)flashAddr, data, readSize );
    if( remainingBankSize < size )
        read( addr + readSize, (uint8_t *)data + readSize, size - readSize );
}

void EEEPROM::erase( uint16_t addr, uint16_t size )
{
    uint8_t *data = (uint8_t *)malloc( size );
    if( data == NULL ) return;
    memset( data, 0xFF, size );
    write( addr, data, size );
    free( data );
}

void EEEPROM::retrieveBankStatus()
{
    for( uint8_t i = 0; i < _numUsableBanks; i++ ) {
        readFlash(
            (void *)( _flashEEEPROMStartAddr + ( i * _minFlashPageSize ) ),
            &_bankStatus[i], 1 );
    }

    _bankUpToDate = true;
}

uint32_t EEEPROM::getNextEmptyBankAddr()
{
    uint32_t addr = _flashEEEPROMStartAddr;

    // Iterate through all banks until we find the next empty bank. This helps
    // with load balancing the various flash banks for a more balanced
    // distribution of writes to the various banks
    if( !_bankUpToDate ) retrieveBankStatus();
    while( _bankStatus[_nextBankUp] != 0xFF ) {
        if( ++_nextBankUp >= _numUsableBanks ) _nextBankUp = 0;
    }

    addr += _nextBankUp * _minFlashPageSize;
    return addr;
}

uint32_t EEEPROM::getFlashAddr( uint16_t eeepromAddr )
{
    uint8_t  bankId = eeepromAddr / _effectivePageSize;
    uint32_t addr = _flashEEEPROMStartAddr;

    // Index through the bank status array, if we get a match use that bank
    // otherwise just use the last empty bank we find.
    if( !_bankUpToDate ) retrieveBankStatus();
    for( uint8_t i = 0; i < _numUsableBanks; i++ ) {
        if( _bankStatus[i] == bankId || _bankStatus[i] == 0xFF ) {
            addr = _flashEEEPROMStartAddr + ( i * _minFlashPageSize ) + 1 +
                   ( eeepromAddr % _effectivePageSize );
            if( _bankStatus[i] == bankId ) break;
        }
    }

    return addr;
}

void EEEPROM::end()
{
    // Take down everything
    free( _bankStatus );
    _bankStatus = NULL;

    _bankUpToDate = false;
    _nextBankUp = 0;
    _EEEPROMSize = 0;
}
