################################################################################
# Arduino SAMD20 Library Makefile
################################################################################

#################################### Setup #####################################
TOP_DIR   := $(dir $(lastword $(MAKEFILE_LIST)))
SDK_ROOT  := $(realpath ..)
PROJ_ROOT := $(realpath .)
HOST      := $(shell hostname)
BUILD_DIR := build

-include $(SDK_ROOT)/config.mk

#################################### Paths #####################################
CMSIS_DIR := $(SDK_ROOT)/devicepack/ARM/CMSIS/$(CMSIS_VER)/$(CMSIS_CORE_INCLUDE)
ATMEL_DFP := $(SDK_ROOT)/devicepack/Atmel/SAMD20_DFP/$(SAMD20_DFP_VER)/$(SAMD20_DFP_INCLUDE)
ARMBIN    ?= $(SDK_ROOT)/toolchain/bin

OUTPUT_FILE_PATH :=$(BUILD_DIR)/libArduinoCore.a
OUTPUT_FILE_PATH_AS_ARGS :=$(BUILD_DIR)/libArduinoCore.a

GCC_VER := $(shell $(ARMBIN)/arm-none-eabi-gcc -dumpversion)
#################################### Files #####################################
# O_SRCS :=
OBJS :=
OBJS_AS_ARGS :=

CSRCS :=
C_DEPS :=
C_DEPS_AS_ARGS :=

PREPROCESSING_SRCS :=
# S_SRCS :=

# Add inputs and outputs from these tool invocations to the build variables
CSRCS = $(wildcard src/*.c)

# Add inputs and outputs from these tool invocations to the build variables
CPPSRCS = $(wildcard src/*.cpp)

# PREPROCESSING_SRCS += $(PROJ_ROOT)/pulse_asm.S

OBJS = $(CSRCS:src/%.c=$(BUILD_DIR)/%.o) $(CPPSRCS:src/%.cpp=$(BUILD_DIR)/%.o)
C_DEPS = $(CSRCS:src/%.c=$(BUILD_DIR)/%.d)

OBJS_AS_ARGS := $(OBJS)
C_DEPS_AS_ARGS := $(C_DEPS)

#################################### Flags #####################################
# CCFLAGS   - general flags that apply to both c and cpp sources
# CFLAGS    - flags that apply only to .c sources files (invoked with gcc)
# CXXFLAGS  - flags that apply only to .cpp sources (invoked with g++)

INCLUDES :=
COMPILER_DEFS :=
CPU :=
CCFLAGS :=
CFLAGS :=
CXXFLAGS :=

INCLUDES += \
	-I$(CMSIS_DIR) \
	-I$(ATMEL_DFP) \
	-I$(PROJ_ROOT)/src

COMPILER_DEFS += \
	-mthumb \
	-D__SAMD20E18__

CPU += -mcpu=cortex-m0plus

CCFLAGS += \
	-DDEBUG \
	-DARDUINO_SAMD_E \
	-DARDUINO_ARCH_SAMD \
	-DSAMD20

CFLAGS += -O3 -ffunction-sections -mlong-calls -g3 -Wall -std=gnu99
CXXFLAGS += -O3 -ffunction-sections -fno-rtti -fno-exceptions -mlong-calls -g3 -Wall

DEPENDS = -MD -MP -MF $(@:%.o=%.d) -MT$(@:%.o=%.d) -MT$(@:%.o=%.o)


################################### Targets ####################################
$(BUILD_DIR)/%.o: $(PROJ_ROOT)/src/%.c
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : $(GCC_VER)
	$(ARMBIN)/arm-none-eabi-gcc $(COMPILER_DEFS) -x c $(CCFLAGS) $(CFLAGS) $(INCLUDES) -c $(CPU) $(DEPENDS) -o $@ $<
	@echo Finished building: $<

$(BUILD_DIR)/%.o: $(PROJ_ROOT)/src/%.cpp
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : $(GCC_VER)
	$(ARMBIN)/arm-none-eabi-g++ $(COMPILER_DEFS) $(CCFLAGS) $(CXXFLAGS) $(INCLUDES) -c $(CPU) $(DEPENDS) -o $@ $<
	@echo Finished building: $<

$(BUILD_DIR)/%.o: $(PROJ_ROOT)/src/%.S
	@echo Building file: $<
	@echo Invoking: ARM/GNU Preprocessing Assembler : $(GCC_VER)
	$(ARMBIN)/arm-none-eabi-gcc $(COMPILER_DEFS) -x assembler-with-cpp -c $(CPU) $(INCLUDES) $(DEPENDS) -Wa,-g -o $@ $<
	@echo Finished building: $<

$(BUILD_DIR)/%.o: $(PROJ_ROOT)/src/%.x
	@echo Building file: $<
	@echo Invoking: ARM/GNU Preprocessing Assembler : $(GCC_VER)
	$(ARMBIN)/arm-none-eabi-gcc $(COMPILER_DEFS) -x assembler-with-cpp -c $(CPU) $(INCLUDES) $(DEPENDS) -Wa,-g -o $@ $<
	@echo Finished building: $<

$(BUILD_DIR)/%.o:: $(PROJ_ROOT)/src/%.X
	@echo Building file: $<
	@echo Invoking: ARM/GNU Preprocessing Assembler : $(GCC_VER)
	$(ARMBIN)/arm-none-eabi-gcc $(COMPILER_DEFS) -x assembler-with-cpp -c $(CPU) $(INCLUDES) $(DEPENDS) -Wa,-g -o $@ $<
	@echo Finished building: $<

$(BUILD_DIR)/%.o:: $(PROJ_ROOT)/src/%.sx
	@echo Building file: $<
	@echo Invoking: ARM/GNU Preprocessing Assembler : $(GCC_VER)
	$(ARMBIN)/arm-none-eabi-gcc $(COMPILER_DEFS) -x assembler-with-cpp -c $(CPU) $(INCLUDES) $(DEPENDS) -Wa,-g -o $@ $<
	@echo Finished building: $<


################################## Directives ##################################
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

.PHONY: prebuild $(OUTPUT_FILE_PATH)

all: prebuild $(OUTPUT_FILE_PATH)

prebuild:
	mkdir -p build

$(OUTPUT_FILE_PATH): $(OBJS)
	@echo Building target: $@
	@echo Invoking: ARM/GNU Archiver : $(GCC_VER)
	$(ARMBIN)/arm-none-eabi-ar -r -o $(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS)
	@echo Finished building target: $@

# Other Targets
clean:
	rm -rf build
