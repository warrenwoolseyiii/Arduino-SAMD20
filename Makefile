################################################################################
# SAM-SDK parent Makefile
################################################################################

# NOTE: See link below for atpack downloads
# http://packs.download.atmel.com/

ATPACK_BASE_URL := "http://packs.download.atmel.com/"

SHELL := /bin/bash

# NOTE: Choose one
# TC_PLATFORM ?= osx
TC_PLATFORM ?= linux

all: .dfp.extracted .toolchain.extracted .arduino.built
	@echo "Done"

.dfp.extracted:
	@$(SHELL) scripts/getpack.sh "$(ATPACK_BASE_URL)" "Atmel" "SAMD20_DFP" "1.2.91"
	@$(SHELL) scripts/getpack.sh "$(ATPACK_BASE_URL)" "ARM" "CMSIS" "5.0.1"
	@touch $@

.toolchain.extracted:
	@$(SHELL) scripts/gettoolchain.sh "$(TC_PLATFORM)"
	@touch $@

.arduino.built:
	@rm -f lib/libArduinoCore.a
	$(MAKE) clean -C arduino
	$(MAKE) -C arduino
	cp arduino/build/libArduinoCore.a lib/libArduinoCore.a
	@touch $@

# Remove all downloads
dl-clean:
	@echo "Removing downloads"
	rm -rf dl

# Remove extracted toolchain components
toolchain-clean:
	@echo "Removing extracted devicepacks and toolchain"
	rm -rf devicepack/ARM devicepack/Atmel
	rm -rf toolchain/arm-none-eabi toolchain/bin toolchain/lib toolchain/share
	rm -f .*.extracted

clean:
	@echo "Cleaning arduino build"
	@rm -f lib/libArduinoCore.a
	$(MAKE) clean -C arduino
	rm -f .arduino.built

distclean: clean toolchain-clean

reallyclean: distclean dl-clean
