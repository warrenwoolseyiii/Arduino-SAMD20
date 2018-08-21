# About
This project was forked from the ArduinoCore-samd project which supports the Arduino core for the SAMD21 family of processors. The code has been refactored to support the SAMD20 family of processors, and has also been altered with low power projects in mind.

# Usage
There are two ways to utilize the SAMD20 Arduino core. All stable release binaries are tagged in github. You can also build the binary from the source and/or modify it for your own usage by cloning this project! 

## Releases
[releases](https://github.com/FlumeTech/Arduino-SAMD20/releases)

## Building
### MakeFile
If you wish to build the library using the provided MakeFile you will need to download the standalone GNU-ARM tool chain, and the Atmel SAM device pack as dependencies. I have provided the [SAM-SDK](https://github.com/FlumeTech/SAM-SDK) dependencies.

1. Clone the SAM-SDK repo
```shell
git clone git@github.com:FlumeTech/SAM-SDK.git
```

2. Clone this repo
```shell
git clone git@github.com:FlumeTech/Arduino-SAMD20.git -b master
```

3. Add the .local directory to your project and the config.mk file
```shell
cd path_to_Arduino-SAMD20/Arduino-SAMD20
mkdir .local
vim config.mk
```

4. Add the path to your SAM-SDK in config.mk
```shell
SDK_PATH := /path/to/SAM-SDK
```

5. Build the project!
```shell
cd path_to_Arduino-SAMD20/Arduino-SAMD20
make all
```

### Atmel Studio Project
TODO:
