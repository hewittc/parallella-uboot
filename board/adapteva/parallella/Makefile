#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# Piggy-back on Xilinx board.
obj-y := board.o ../../xilinx/zynq/board.o

hw-platform-y :=$(shell echo $(CONFIG_DEFAULT_DEVICE_TREE))

# We need I2C to configure the PMIC. I2C is hooked up via EMIO so we need to
# have the PL initialized *before* we can configure voltage levels in U-BOOT.
# So it's hard for us to support SPL.

init-objs := $(if $(wildcard $(srctree)/$(src)/$(hw-platform-y)/ps7_init_gpl.c),\
	$(hw-platform-y)/ps7_init_gpl.o)

ifeq ($(init-objs),)
ifneq ($(wildcard $(srctree)/$(src)/ps7_init_gpl.c),)
init-objs := ps7_init_gpl.o
$(if $(CONFIG_SPL_BUILD),\
$(warning Put custom ps7_init_gpl.c/h to board/adapteva/parallella/))
endif
endif

obj-$(CONFIG_SPL_BUILD) += $(init-objs)

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_ps7_init_gpl.o := -Wstrict-prototypes

# To include xil_io.h
CFLAGS_ps7_init_gpl.o := -I$(srctree)/board/xilinx/zynq
