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

#include "SysTick.h"
#include "atomic.h"

volatile uint64_t            _sysTickUnderFlows = 0;
volatile uint8_t             _tickIsInit = 0;
volatile uint8_t             _useUpdatedCnt = 0;
volatile CPUTix_Debug_t      _tixDBG = {0, 0, 0, 0, 0};
volatile DelayCPUTix_Debug_t _delayTixDBG = {0, 0, 0, 0, 0};

void SysTick_IRQHandler()
{
    if( ( _sysTickUnderFlows++ ) > 0xFFFFFFFFFF ) _sysTickUnderFlows = 0;

    // If we are inside the GET_CPU_TICKS macro then we need to update the
    // externally available _useUpdatedCnt field and alert the GET_CPU_TICKS
    // macro that an overflow has occurred. When we re-enter GET_CPU_TICKS the
    // updated under flow counter and new register value should be used.
    if( _tixDBG.insideGet ) _useUpdatedCnt = 1;
}

void initSysTick()
{
    if( !_tickIsInit ) {
        ATOMIC_OPERATION( {
            SysTick->CTRL = 0;
            SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
            SysTick_Config( SYS_TICK_UNDERFLOW ); // Max value is 2^24 ticks
            _sysTickUnderFlows = 0;
            _useUpdatedCnt = 0;
            _tixDBG.insideGet = 0;
            _tickIsInit = 1;
        } )
    }
}

void disableSysTick()
{
    if( _tickIsInit ) {
        ATOMIC_OPERATION( {
            SysTick->CTRL = 0;
            SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
            _tickIsInit = 0;
        } )
    }
}

// WARNING: In the deepest sleep mode (STANDBY) the CPU is disabled and the
// SysTick counter is disabled. Upon waking the CPU ticks counter will be
// completely restarted. If you wish to use a timing mechanism that persists
// through STANDBY sleep mode use the stepsRTC() functionality.
uint64_t getCPUTicks()
{
    // Set returned to 0
    _tixDBG.inside = 1;
    _tixDBG.tixReturned = 0;

    // Check if the system tick module is initialized
    _tixDBG.hadToInit = !_tickIsInit;
    if( _tixDBG.hadToInit ) initSysTick();

    // Place the soft lock on the process
    _tixDBG.insideGet = 1;
    _tixDBG.tixReturned = ( ( _sysTickUnderFlows << 24 ) |
                            ( SYS_TICK_UNDERFLOW - SysTick->VAL ) );

    // Check if we need to use the updated count
    _tixDBG.usedUpdatedCount = _useUpdatedCnt;
    if( _tixDBG.usedUpdatedCount ) {
        _tixDBG.tixReturned = ( ( _sysTickUnderFlows << 24 ) |
                                ( SYS_TICK_UNDERFLOW - SysTick->VAL ) );
        _useUpdatedCnt = 0;
    }

    // Remove the soft lock
    _tixDBG.insideGet = 0;
    _tixDBG.inside = 0;

    return _tixDBG.tixReturned;
}

void delayCPUTicks( uint64_t tix )
{
    // Set the duration
    _delayTixDBG.inside = 1;
    _delayTixDBG.tix = tix;

    // Check if the system tick module is initialized
    _delayTixDBG.hadToInit = !_tickIsInit;
    if( _delayTixDBG.hadToInit ) initSysTick();

    // Get the start time and burn until we reached tix
    _delayTixDBG.start = getCPUTicks();
    do {
        _delayTixDBG.cnt = getCPUTicks();
    } while( ( _delayTixDBG.cnt - _delayTixDBG.start ) < _delayTixDBG.tix );

    _delayTixDBG.inside = 0;
}

void getCPUTixDebugInfo( CPUTix_Debug_t *tix, DelayCPUTix_Debug_t *dTix,
                         uint8_t *init )
{
    tix->hadToInit = _tixDBG.hadToInit;
    tix->inside = _tixDBG.inside;
    tix->insideGet = _tixDBG.insideGet;
    tix->tixReturned = _tixDBG.tixReturned;
    tix->usedUpdatedCount = _tixDBG.usedUpdatedCount;

    dTix->cnt = _delayTixDBG.cnt;
    dTix->hadToInit = _delayTixDBG.hadToInit;
    dTix->inside = _delayTixDBG.inside;
    dTix->start = _delayTixDBG.start;
    dTix->tix = _delayTixDBG.tix;

    *init = _tickIsInit;
}
