### Download/extract directory
### Contains all toolchain/devicepack downloads
# Use a folder outside of this sdk project directory (prevents re-downloading)
# DL_DIR ?= $(shell realpath ..)/sam-sdk-dl
# Use a folder within this this project directory (okay for developers)
DL_DIR ?= $(shell realpath .)/dl

### AMR toolchain version: Choose one ###
# TC_VER ?= 7-2018-q2-update
# TC_VER ?= 8-2019-q3-update
TC_VER ?= 9-2019-q4-major

### Toolchain platform: Choose one ###
TC_PLATFORM ?= linux
# TC_PLATFORM ?= mac

# Pack versions here: http://packs.download.atmel.com/
# NOTE: Atmel likes to change their paths, so relative include paths
# must also be defined below.

### SAMD20_DFP version and include path ###
# SAMD20_DFP_VER ?= 1.2.91
SAMD20_DFP_VER ?= 1.3.124
SAMD20_DFP_INCLUDE ?= samd20/include

### ARM CMSIS version and include path ###
# CMSIS_VER      ?= 5.0.1
# CMSIS_CORE_INCLUDE ?= CMSIS/Include
CMSIS_VER      ?= 5.4.0
CMSIS_CORE_INCLUDE ?= CMSIS/Core/Include
