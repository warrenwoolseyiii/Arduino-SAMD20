#include <Arduino.h>

#define FLASH_SS 10
#define RFM_SS 7
#define RFM_INT 15
#define FXOS_CS_PIN 11
#define FXOS_SCK_PIN 20
#define FXOS_MOSI_PIN 19
#define FXOS_MISO_PIN 2
#define FXOS_RST_PIN 3
#define FXOS_INT1_PIN 13
#define FXOS_INT2_PIN 14
#define LED1 4
#define LED2 12

uint8_t p1, p2;

void setup()
{
    p1 = 0;
    p2 = 1;
    pinMode( FXOS_INT1_PIN, OUTPUT );
    pinMode( FXOS_INT2_PIN, OUTPUT );
    changeCPUClk( cpu_clk_dfll48 );

    digitalWrite( FXOS_INT1_PIN, p1 );
    digitalWrite( FXOS_INT2_PIN, p2 );
}

void loop()
{
    p1 ^= 0x1;
    p2 ^= 0x1;
    digitalWrite( FXOS_INT1_PIN, p1 );
    delayMicroseconds( 100 );
    digitalWrite( FXOS_INT2_PIN, p2 );

    p1 ^= 0x1;
    p2 ^= 0x1;
    delay( 1 );
    digitalWrite( FXOS_INT1_PIN, p1 );
    delay( 1 );
    digitalWrite( FXOS_INT2_PIN, p2 );
    delay( 5 );
}
