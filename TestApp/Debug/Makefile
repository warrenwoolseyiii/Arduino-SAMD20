################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../Device_Startup/


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../../../Flume/Flume_AMR/lib/Commands/CmdImp.cpp \
../../../Flume/Flume_AMR/lib/CurrentValueTable/CVT.cpp \
../../../Flume/Flume_AMR/lib/Flume/FlumeID.cpp \
../../../Flume/Flume_AMR/lib/FXOS8700/filter.cpp \
../../../Flume/Flume_AMR/lib/FXOS8700/FXOS8700.cpp \
../../../Flume/Flume_AMR/lib/SPIFlash/SPIFlash.cpp \
../../../Flume/Flume_AMR/src/Flume_AMR.cpp


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
CmdImp.o \
CVT.o \
FlumeID.o \
filter.o \
FXOS8700.o \
SPIFlash.o \
Flume_AMR.o

OBJS_AS_ARGS +=  \
CmdImp.o \
CVT.o \
FlumeID.o \
filter.o \
FXOS8700.o \
SPIFlash.o \
Flume_AMR.o

C_DEPS +=  \
CmdImp.d \
CVT.d \
FlumeID.d \
filter.d \
FXOS8700.d \
SPIFlash.d \
Flume_AMR.d

C_DEPS_AS_ARGS +=  \
CmdImp.d \
CVT.d \
FlumeID.d \
filter.d \
FXOS8700.d \
SPIFlash.d \
Flume_AMR.d

OUTPUT_FILE_PATH +=TestApp.elf

OUTPUT_FILE_PATH_AS_ARGS +=TestApp.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+=  \
C:/Users/budge/Repositories/Arduino-SAMD20/SAMD20E18/ArduinoCore/Debug/libArduinoCore.a

LINKER_SCRIPT_DEP+=  \
../Device_Startup/samd20j18_flash.ld \
../Device_Startup/samd20j18_sram.ld


# AVR32/GNU C Compiler















./%.o: ../../../Flume/Flume_AMR/lib/Commands/%.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAMD20E18__ -DDEBUG -DF_CPU=48000000L -DARDUINO_ARCH_SAMD -DSAMD20 -DFLUME_BUILD_TYPE=1 -DSERIAL_EN=1 -DINT_DEBUG_EN=0 -DDEBUG_CONSOLE_EN=1 -DTIMERTEST_EN=1 -DUUID_BINLEN=8 -DFXOS_EN=1 -DFXOS_I2C_ADDR=0x1E -DFXOS_ODR=ODR_200HZ -DFXOS_OSR=MOSR_6 -DFXOS_SLEEP_OSR=MSOSR_4 -DFXOS_STREAMING=0 -DRX_NO_PROCESSES=0 -DFLASH_EN=1 -DLED_EN=1  -I"../../SAMD20E18/ArduinoCore/include/core" -I"../../SAMD20E18/ArduinoCore/include/core/avr" -I"../../SAMD20E18/ArduinoCore/include/core/samd20" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/component" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/instance" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/pio" -I"../../SAMD20E18/ArduinoCore/include/libraries" -I"../../SAMD20E18/ArduinoCore/include/libraries/spi" -I"../../SAMD20E18/ArduinoCore/include/libraries/wire" -I"../../SAMD20E18/ArduinoCore/include/variants" -I"../../../Flume/Flume_AMR" -I"../../../Flume/Flume_AMR/lib/include" -I"../../../Flume/Flume_AMR/lib/Flume" -I"../../../Flume/Flume_AMR/lib/FXOS8700" -I"../../../Flume/Flume_AMR/lib/SPIFlash" -I"../../../Flume/Flume_AMR/lib/Commands" -I"../../../Flume/Flume_AMR/lib/CurrentValueTable" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAMD20_DFP\1.2.91\samd20\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\5.0.1\CMSIS\Include"  -O1 -ffunction-sections -fno-rtti -fno-exceptions -mlong-calls -g3 -Wall -mcpu=cortex-m0plus -c -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

./%.o: ../../../Flume/Flume_AMR/lib/CurrentValueTable/%.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAMD20E18__ -DDEBUG -DF_CPU=48000000L -DARDUINO_ARCH_SAMD -DSAMD20 -DFLUME_BUILD_TYPE=1 -DSERIAL_EN=1 -DINT_DEBUG_EN=0 -DDEBUG_CONSOLE_EN=1 -DTIMERTEST_EN=1 -DUUID_BINLEN=8 -DFXOS_EN=1 -DFXOS_I2C_ADDR=0x1E -DFXOS_ODR=ODR_200HZ -DFXOS_OSR=MOSR_6 -DFXOS_SLEEP_OSR=MSOSR_4 -DFXOS_STREAMING=0 -DRX_NO_PROCESSES=0 -DFLASH_EN=1 -DLED_EN=1  -I"../../SAMD20E18/ArduinoCore/include/core" -I"../../SAMD20E18/ArduinoCore/include/core/avr" -I"../../SAMD20E18/ArduinoCore/include/core/samd20" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/component" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/instance" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/pio" -I"../../SAMD20E18/ArduinoCore/include/libraries" -I"../../SAMD20E18/ArduinoCore/include/libraries/spi" -I"../../SAMD20E18/ArduinoCore/include/libraries/wire" -I"../../SAMD20E18/ArduinoCore/include/variants" -I"../../../Flume/Flume_AMR" -I"../../../Flume/Flume_AMR/lib/include" -I"../../../Flume/Flume_AMR/lib/Flume" -I"../../../Flume/Flume_AMR/lib/FXOS8700" -I"../../../Flume/Flume_AMR/lib/SPIFlash" -I"../../../Flume/Flume_AMR/lib/Commands" -I"../../../Flume/Flume_AMR/lib/CurrentValueTable" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAMD20_DFP\1.2.91\samd20\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\5.0.1\CMSIS\Include"  -O1 -ffunction-sections -fno-rtti -fno-exceptions -mlong-calls -g3 -Wall -mcpu=cortex-m0plus -c -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

./%.o: ../../../Flume/Flume_AMR/lib/Flume/%.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAMD20E18__ -DDEBUG -DF_CPU=48000000L -DARDUINO_ARCH_SAMD -DSAMD20 -DFLUME_BUILD_TYPE=1 -DSERIAL_EN=1 -DINT_DEBUG_EN=0 -DDEBUG_CONSOLE_EN=1 -DTIMERTEST_EN=1 -DUUID_BINLEN=8 -DFXOS_EN=1 -DFXOS_I2C_ADDR=0x1E -DFXOS_ODR=ODR_200HZ -DFXOS_OSR=MOSR_6 -DFXOS_SLEEP_OSR=MSOSR_4 -DFXOS_STREAMING=0 -DRX_NO_PROCESSES=0 -DFLASH_EN=1 -DLED_EN=1  -I"../../SAMD20E18/ArduinoCore/include/core" -I"../../SAMD20E18/ArduinoCore/include/core/avr" -I"../../SAMD20E18/ArduinoCore/include/core/samd20" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/component" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/instance" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/pio" -I"../../SAMD20E18/ArduinoCore/include/libraries" -I"../../SAMD20E18/ArduinoCore/include/libraries/spi" -I"../../SAMD20E18/ArduinoCore/include/libraries/wire" -I"../../SAMD20E18/ArduinoCore/include/variants" -I"../../../Flume/Flume_AMR" -I"../../../Flume/Flume_AMR/lib/include" -I"../../../Flume/Flume_AMR/lib/Flume" -I"../../../Flume/Flume_AMR/lib/FXOS8700" -I"../../../Flume/Flume_AMR/lib/SPIFlash" -I"../../../Flume/Flume_AMR/lib/Commands" -I"../../../Flume/Flume_AMR/lib/CurrentValueTable" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAMD20_DFP\1.2.91\samd20\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\5.0.1\CMSIS\Include"  -O1 -ffunction-sections -fno-rtti -fno-exceptions -mlong-calls -g3 -Wall -mcpu=cortex-m0plus -c -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

./%.o: ../../../Flume/Flume_AMR/lib/FXOS8700/%.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAMD20E18__ -DDEBUG -DF_CPU=48000000L -DARDUINO_ARCH_SAMD -DSAMD20 -DFLUME_BUILD_TYPE=1 -DSERIAL_EN=1 -DINT_DEBUG_EN=0 -DDEBUG_CONSOLE_EN=1 -DTIMERTEST_EN=1 -DUUID_BINLEN=8 -DFXOS_EN=1 -DFXOS_I2C_ADDR=0x1E -DFXOS_ODR=ODR_200HZ -DFXOS_OSR=MOSR_6 -DFXOS_SLEEP_OSR=MSOSR_4 -DFXOS_STREAMING=0 -DRX_NO_PROCESSES=0 -DFLASH_EN=1 -DLED_EN=1  -I"../../SAMD20E18/ArduinoCore/include/core" -I"../../SAMD20E18/ArduinoCore/include/core/avr" -I"../../SAMD20E18/ArduinoCore/include/core/samd20" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/component" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/instance" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/pio" -I"../../SAMD20E18/ArduinoCore/include/libraries" -I"../../SAMD20E18/ArduinoCore/include/libraries/spi" -I"../../SAMD20E18/ArduinoCore/include/libraries/wire" -I"../../SAMD20E18/ArduinoCore/include/variants" -I"../../../Flume/Flume_AMR" -I"../../../Flume/Flume_AMR/lib/include" -I"../../../Flume/Flume_AMR/lib/Flume" -I"../../../Flume/Flume_AMR/lib/FXOS8700" -I"../../../Flume/Flume_AMR/lib/SPIFlash" -I"../../../Flume/Flume_AMR/lib/Commands" -I"../../../Flume/Flume_AMR/lib/CurrentValueTable" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAMD20_DFP\1.2.91\samd20\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\5.0.1\CMSIS\Include"  -O1 -ffunction-sections -fno-rtti -fno-exceptions -mlong-calls -g3 -Wall -mcpu=cortex-m0plus -c -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

./%.o: ../../../Flume/Flume_AMR/lib/SPIFlash/%.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAMD20E18__ -DDEBUG -DF_CPU=48000000L -DARDUINO_ARCH_SAMD -DSAMD20 -DFLUME_BUILD_TYPE=1 -DSERIAL_EN=1 -DINT_DEBUG_EN=0 -DDEBUG_CONSOLE_EN=1 -DTIMERTEST_EN=1 -DUUID_BINLEN=8 -DFXOS_EN=1 -DFXOS_I2C_ADDR=0x1E -DFXOS_ODR=ODR_200HZ -DFXOS_OSR=MOSR_6 -DFXOS_SLEEP_OSR=MSOSR_4 -DFXOS_STREAMING=0 -DRX_NO_PROCESSES=0 -DFLASH_EN=1 -DLED_EN=1  -I"../../SAMD20E18/ArduinoCore/include/core" -I"../../SAMD20E18/ArduinoCore/include/core/avr" -I"../../SAMD20E18/ArduinoCore/include/core/samd20" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/component" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/instance" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/pio" -I"../../SAMD20E18/ArduinoCore/include/libraries" -I"../../SAMD20E18/ArduinoCore/include/libraries/spi" -I"../../SAMD20E18/ArduinoCore/include/libraries/wire" -I"../../SAMD20E18/ArduinoCore/include/variants" -I"../../../Flume/Flume_AMR" -I"../../../Flume/Flume_AMR/lib/include" -I"../../../Flume/Flume_AMR/lib/Flume" -I"../../../Flume/Flume_AMR/lib/FXOS8700" -I"../../../Flume/Flume_AMR/lib/SPIFlash" -I"../../../Flume/Flume_AMR/lib/Commands" -I"../../../Flume/Flume_AMR/lib/CurrentValueTable" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAMD20_DFP\1.2.91\samd20\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\5.0.1\CMSIS\Include"  -O1 -ffunction-sections -fno-rtti -fno-exceptions -mlong-calls -g3 -Wall -mcpu=cortex-m0plus -c -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

./%.o: ../../../Flume/Flume_AMR/src/%.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -mthumb -D__SAMD20E18__ -DDEBUG -DF_CPU=48000000L -DARDUINO_ARCH_SAMD -DSAMD20 -DFLUME_BUILD_TYPE=1 -DSERIAL_EN=1 -DINT_DEBUG_EN=0 -DDEBUG_CONSOLE_EN=1 -DTIMERTEST_EN=1 -DUUID_BINLEN=8 -DFXOS_EN=1 -DFXOS_I2C_ADDR=0x1E -DFXOS_ODR=ODR_200HZ -DFXOS_OSR=MOSR_6 -DFXOS_SLEEP_OSR=MSOSR_4 -DFXOS_STREAMING=0 -DRX_NO_PROCESSES=0 -DFLASH_EN=1 -DLED_EN=1  -I"../../SAMD20E18/ArduinoCore/include/core" -I"../../SAMD20E18/ArduinoCore/include/core/avr" -I"../../SAMD20E18/ArduinoCore/include/core/samd20" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/component" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/instance" -I"../../SAMD20E18/ArduinoCore/include/core/samd20/include/pio" -I"../../SAMD20E18/ArduinoCore/include/libraries" -I"../../SAMD20E18/ArduinoCore/include/libraries/spi" -I"../../SAMD20E18/ArduinoCore/include/libraries/wire" -I"../../SAMD20E18/ArduinoCore/include/variants" -I"../../../Flume/Flume_AMR" -I"../../../Flume/Flume_AMR/lib/include" -I"../../../Flume/Flume_AMR/lib/Flume" -I"../../../Flume/Flume_AMR/lib/FXOS8700" -I"../../../Flume/Flume_AMR/lib/SPIFlash" -I"../../../Flume/Flume_AMR/lib/Commands" -I"../../../Flume/Flume_AMR/lib/CurrentValueTable" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAMD20_DFP\1.2.91\samd20\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\5.0.1\CMSIS\Include"  -O1 -ffunction-sections -fno-rtti -fno-exceptions -mlong-calls -g3 -Wall -mcpu=cortex-m0plus -c -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: ARM/GNU Linker : 6.3.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-g++.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -mthumb -Wl,-Map="TestApp.map" --specs=nosys.specs -Wl,--start-group -lm -lArduinoCore  -Wl,--end-group -L"..\\Device_Startup" -L"C:\Users\budge\Repositories\Arduino-SAMD20\SAMD20E18\ArduinoCore\Debug"  -Wl,--gc-sections -mcpu=cortex-m0plus -Tsamd20j18_flash.ld  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O binary "TestApp.elf" "TestApp.bin"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "TestApp.elf" "TestApp.hex"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O binary "TestApp.elf" "TestApp.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objdump.exe" -h -S "TestApp.elf" > "TestApp.lss"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature  "TestApp.elf" "TestApp.srec"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-size.exe" "TestApp.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "TestApp.elf" "TestApp.a" "TestApp.hex" "TestApp.bin" "TestApp.lss" "TestApp.eep" "TestApp.map" "TestApp.srec"
	