# About
This project was forked from the ArduinoCore-samd project which supports the Arduino core for the SAMD21 family of processors. The code has been refactored to support the SAMD20 family of processors, and has also been altered with low power projects in mind.
# How to build
Run `make help` for all make options
Open the `config.mk` file and adjust the `DL_DIR` path if necessary, default is just a folder inside the repo called `dl`. You can also adjust the Atmel DFP and CMSIS core versions from this file.
```
make distclean
make
```
# Rebuilding
If you don't want to re-download the DFP and CMSIS distro, just run `make clean` and then `make` to rebuild from the arduino source files.
