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

volatile uint32_t fxos = 0;
volatile uint32_t rfm = 0;

void fxosISR()
{
    fxos++;
    digitalWrite( LED1, LOW );
    delay( 1 );
    digitalWrite( LED1, HIGH );
}

void rfmISR()
{
    rfm++;
    digitalWrite( LED2, LOW );
    delay( 1 );
    digitalWrite( LED2, HIGH );
}

void setup()
{
    pinMode( LED1, OUTPUT );
    pinMode( LED2, OUTPUT );

    digitalWrite( LED1, HIGH );
    digitalWrite( LED2, HIGH );

    changeCPUClk( cpu_clk_oscm8 );
    attachInterrupt( FXOS_INT2_PIN, fxosISR, FALLING );
    attachInterrupt( RFM_INT, rfmISR, RISING );
}

void loop()
{
    interruptlowPowerMode( true );
    delay( 1000 );
    __disable_irq();
    for( uint32_t i = 0; i < 100000; i++ )
        ;
    __enable_irq();
}
