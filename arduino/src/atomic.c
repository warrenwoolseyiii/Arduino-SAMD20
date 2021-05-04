#include "atomic.h"

uint8_t  _atomicNestCount = 0;
uint32_t _atomicReEnableFlags = 0;

void startAtomicOperation()
{
    // Block every interrupt except the watchdog timer
    ATOMIC_OPERATION( {
        if( _atomicNestCount == 0 ) {
            // Disable the ARM systick
            NVIC_DisableIRQ( SysTick_IRQn );

            // Loop through all processor based interrupts and disable them
            for( IRQn_Type i = PM_IRQn; i < PERIPH_COUNT_IRQn; i++ ) {
                if( NVIC_GetEnableIRQ( i ) && i != WDT_IRQn ) {
                    _atomicReEnableFlags |= ( 1UL << i );
                    NVIC_DisableIRQ( i );
                }
            }
        }

        _atomicNestCount++;
    } )
}

void endAtomicOperation()
{
    ATOMIC_OPERATION( {
        if( _atomicNestCount > 0 ) _atomicNestCount--;
        if( _atomicNestCount == 0 ) {
            // Enable ARM systick
            NVIC_EnableIRQ( SysTick_IRQn );

            // Loop through all processor based interrupts and enable the ones
            // that were enabled
            for( IRQn_Type i = PM_IRQn; i < PERIPH_COUNT_IRQn; i++ ) {
                if( _atomicReEnableFlags & ( 1UL << i ) ) {
                    _atomicReEnableFlags &= ~( 1UL << i );
                    NVIC_EnableIRQ( i );
                }
            }
        }
    } )
}
