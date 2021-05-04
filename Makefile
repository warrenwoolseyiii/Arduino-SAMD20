################################################################################
# SAM-SDK parent Makefile
################################################################################

# NOTE: See link below for atpack downloads
# http://packs.download.atmel.com/

ATPACK_BASE_URL := "http://packs.download.atmel.com/"

SHELL := /bin/bash

HOST = $(shell hostname -s)

ifeq ($(HOST),deploy)
include ci.mk
endif
include config.mk
include checks.mk

.PHONY: relink arduino help

all: .cmsis.$(CMSIS_VER).extracted .dfp.$(SAMD20_DFP_VER).extracted .toolchain.$(TC_VER).extracted relink arduino
	@echo "Done"

.dfp.$(SAMD20_DFP_VER).extracted:
	($(SHELL) scripts/getpack.sh "$(ATPACK_BASE_URL)" "Atmel" "SAMD20_DFP" "$(SAMD20_DFP_VER)" "$(DL_DIR)" || exit 1);
	@touch $@

.cmsis.$(CMSIS_VER).extracted:
	($(SHELL) scripts/getpack.sh "$(ATPACK_BASE_URL)" "ARM" "CMSIS" "$(CMSIS_VER)" "$(DL_DIR)" || exit 1);
	@touch $@

.toolchain.$(TC_VER).extracted:
	@echo "USING TC_VER=$(TC_VER)"
	($(SHELL) scripts/gettoolchain.sh "$(TC_VER)" "$(TC_PLATFORM)" "$(DL_DIR)" || exit 1);
	@touch $@

relink:
	($(SHELL) scripts/relink.sh "$(TC_VER)" "$(DL_DIR)" || exit 1);

arduino:
	@rm -f lib/libArduinoCore.a
	$(MAKE) clean -C arduino
	$(MAKE) -C arduino
	cp arduino/build/libArduinoCore.a lib/libArduinoCore.a

# Remove extracted toolchain components
clean-cur-toolchain:
	@echo "Removing toolchain $(TC_VER), CMSIS $(CMSIS_VER), DFP $(SAMD20_DFP_VER)"
	rm -rf toolchains/$(TC_VER)
	($(SHELL) -c "[[ -L toolchain ]] && unlink toolchain || exit 0");
	rm -rf devicepack/ARM/CMSIS/$(CMSIS_VER) devicepack/Atmel/SAMD20_DFP/$(SAMD20_DFP_VER)
	rm -f .cmsis.$(CMSIS_VER).*
	rm -f .dfp.$(SAMD20_DFP_VER).*
	rm -f .toolchain.$(TC_VER).*

clean-all-toolchains:
	@echo "Removing all extracted toolchain components"
	rm -rf toolchains
	rm -f .toolchain.*
	rm -f .cmsis.*
	rm -f .dfp.*
	($(SHELL) -c "[[ -L toolchain ]] && unlink toolchain || exit 0");

clean:
	@echo "Cleaning arduino build"
	@rm -f lib/libArduinoCore.a
	$(MAKE) clean -C arduino

distclean: clean clean-all-toolchains

help:
	@echo ""
	@echo "Run \"make\" to collect toolchains, device packs (set in config.mk), then build arduino"
	@echo "Once toolchain is set up, running make again just rebuilds arduino"
	@echo ""
	@echo "Other targets"
	@echo "clean:                clean arduino build"
	@echo "clean-cur-toolchain:  remove the current toolchain components (set by config.mk)"
	@echo "clean-all-toolchains: remove all extracted toolchain components"
	@echo "distclean:            clean + clean-all-toolchains"
