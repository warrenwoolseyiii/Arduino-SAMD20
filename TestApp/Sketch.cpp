#include <Arduino.h>
#include <lib/include/Debug.h>
#include <lib/SPIFlash/SPIFlash.h>

#if defined(__SAMD20E18__)
#define FLASH_SS 10
#define RFM_SS 7
#define LED1 4
#define LED2 9 
#elif defined(__SAMD20J18__)
#define FLASH_SS 2
#define RFM_SS 7
#define LED1 LED_BUILTIN
#define LED2 9
#endif

uint8_t gLevel = 0x1;
uint32_t gAddr = 0;
SPIFlash Flash(FLASH_SS);

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(RFM_SS, OUTPUT);
  digitalWrite(RFM_SS, HIGH);
  digitalWrite(LED1, HIGH);
  //Flash.initialize();
  //Flash.blockErase32K(gAddr);
  //DEBUGbegin(SERIAL_BAUD);
}

void loop() {
  digitalWrite(LED1, gLevel);
  delay(500);
  uint32_t ms = millis();
  //DEBUGln(ms);
  //Flash.writeBytes(gAddr, &ms, sizeof(ms));
  
  //uint32_t test = 0;
  //Flash.readBytes(gAddr, &test, sizeof(test));
  //if( test == ms )  
  gLevel ^= 0x1;
  gAddr += sizeof(uint32_t);
}