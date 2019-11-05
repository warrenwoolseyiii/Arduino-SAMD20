# About
This project was forked from the ArduinoCore-samd project which supports the Arduino core for the SAMD21 family of processors. The code has been refactored to support the SAMD20 family of processors, and has also been altered with low power projects in mind.

# Usage
Build and linke the .a library against your adrduino project (some libraries have been integrated locally such as SPI).
Enter the top level directory and use `make` to build the library. If you build from a clean build, the make file should automatically grab the latest toolchain and device pack from microchip.
