#ifndef ATOMIC_H_
#define ATOMIC_H_

#include <stdint.h>
#include "sam.h"

#define ATOMIC_OPERATION( X )                   \
    {                                           \
        uint32_t _atomicPrim = __get_PRIMASK(); \
        __disable_irq();                        \
        {                                       \
            X                                   \
        }                                       \
        if( !_atomicPrim ) {                    \
            __enable_irq();                     \
        }                                       \
    }

#ifdef __cplusplus
extern "C" {
#endif

void startAtomicOperation();
void endAtomicOperation();

#ifdef __cplusplus
}
#endif

#endif /* ATOMIC_H_ */
