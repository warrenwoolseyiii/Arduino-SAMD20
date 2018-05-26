#include <Arduino.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
}

uint8_t gLevel = 0x1;
void loop() {
  digitalWrite(LED_BUILTIN, gLevel);
  gLevel ^= 0x1;
  delay(500);
  Serial.print(millis());
}