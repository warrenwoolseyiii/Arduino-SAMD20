# About
This project was forked from the ArduinoCore-samd project which supports the Arduino core for the SAMD21 family of processors. The code has been refactored to support the SAMD20 family of processors, and has also been altered with low power projects in mind.

# Dependencies for Ubuntu 18.04
sudo apt install unzip

# How to build
Run `make help` for all make options
Open the `config.mk` file and adjust the `DL_DIR` path if necessary, default is just a folder inside the repo called `dl`. You can also adjust the Atmel DFP and CMSIS core versions from this file.
```
make distclean
make
```

**Note for Ubuntu 18.04** : `sudo` might have to issued with the make commands for it to work. If rebuilding, it is worth issuing `sudo make distclean` instead of just `sudo make clean` as one can face `core_cm0plus.h: No such file or directory` error

# Rebuilding
If you don't want to re-download the DFP and CMSIS distro, just run `make clean` and then `make` to rebuild from the arduino source files.
