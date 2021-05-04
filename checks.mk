ifeq ($(TC_VER),)
$(error "missing TC_VER. See config.mk.")
endif
ifeq ($(TC_PLATFORM),)
$(error "missing TC_PLATFORM. See config.mk.")
endif
ifeq ($(SAMD20_DFP_VER),)
$(error "missing SAMD20_DFP_VER. See config.mk.")
endif
ifeq ($(SAMD20_DFP_INCLUDE),)
$(error "missing SAMD20_DFP_INCLUDE. See config.mk.")
endif
ifeq ($(CMSIS_VER),)
$(error "missing CMSIS_VER. See config.mk.")
endif
ifeq ($(CMSIS_CORE_INCLUDE),)
$(error "missing CMSIS_CORE_INCLUDE. See config.mk.")
endif
