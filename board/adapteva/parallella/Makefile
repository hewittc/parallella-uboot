#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# Piggy-back on Xilinx board.
obj-y := board.o ../../xilinx/zynq/board.o

# We need I2C to configure the PMIC. I2C is hooked up via EMIO so we need to
# have the PL initialized *before* we can configure voltage levels in U-BOOT.
# So it's hard for us to support SPL.

# Please copy ps7_init_gpl.c/h from hw project to this directory
obj-$(CONFIG_SPL_BUILD) += \
		$(if $(wildcard $(srctree)/$(src)/ps7_init_gpl.c), ps7_init_gpl.o, \
			$(if $(wildcard $(srctree)/$(src)/ps7_init.c), ps7_init.o legacy.o))

# Suppress "warning: function declaration isn't a prototype"
CFLAGS_REMOVE_ps7_init_gpl.o := -Wstrict-prototypes
CFLAGS_REMOVE_ps7_init.o := -Wstrict-prototypes
