#include <stdint.h>

#define USER_WORD_0 (uint32_t)0xD9FEC7FF
#define USER_WORD_1 (uint32_t)0xFFFF3F5D

__attribute__ ((section(".user_word"))) const uint32_t settings[] = {USER_WORD_0 ,USER_WORD_1};
