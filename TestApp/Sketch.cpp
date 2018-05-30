#include <Arduino.h>
#include <lib/SPIFlash/SPIFlash.h>

uint8_t gLevel = 0x1;
uint32_t gAddr = 0;
SPIFlash Flash(2);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(500000);
  Flash.initialize();
  Flash.blockErase32K(gAddr);
}

void loop() {
  digitalWrite(LED_BUILTIN, gLevel);
  delay(500);
  uint32_t ms = millis();
  Serial.println(ms);
  Flash.writeBytes(gAddr, &ms, sizeof(ms));
  
  uint32_t test = 0;
  Flash.readBytes(gAddr, &test, sizeof(test));
  if( test == ms )  gLevel ^= 0x1;
  gAddr += sizeof(uint32_t);
}