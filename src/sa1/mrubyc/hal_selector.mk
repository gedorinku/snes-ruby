#
# mruby/c  src/hal_selector.mk
#
# Copyright (C) 2015-2021 Kyushu Institute of Technology.
# Copyright (C) 2015-2021 Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#

ifdef MRBC_USE_HAL
  HAL_DIR = $(MRBC_USE_HAL)
  CFLAGS += -I$(MRBC_USE_HAL)
endif

ifdef MRBC_USE_HAL_POSIX
  HAL_DIR = hal_posix
  CFLAGS += -DMRBC_USE_HAL_POSIX
endif
ifdef MRBC_USE_HAL_PSOC5LP
  HAL_DIR = hal_psoc5lp
endif
ifdef MRBC_USE_HAL_ESP32
  HAL_DIR = hal_esp32
endif
ifdef MRBC_USE_HAL_PIC24
  HAL_DIR = hal_pic24
endif
ifdef MRBC_USE_HAL_RP2040
   HAL_DIR = hal_rp2040
endif

ifndef HAL_DIR
  HAL_DIR = hal_posix
  CFLAGS += -DMRBC_USE_HAL_POSIX
endif
