/*
 * Copyright (C) 2015 Adapteva, Inc.
 *
 * Author: Ola Jeppsson <ola@adapteva.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <dm/uclass.h>
#include <asm/io.h>
#include <fpga.h>
#include <xilinx.h>

DECLARE_GLOBAL_DATA_PTR;

#define ISL9305_I2C_ADDR 0x68 /* PMIC */

static inline int isl9305_probe(struct udevice **i2c)
{
#ifdef CONFIG_DM_I2C
	struct udevice *dev;
	struct udevice *chip;
	int ret;

        ret = uclass_get_device(UCLASS_I2C, 0, &dev);
        if (ret) {
		debug("isl9305_probe: Could not get I2C device (err = %d)\n", ret);
		return ret;
	}

	ret = dm_i2c_probe(dev, ISL9305_I2C_ADDR, 0, &chip);
	if (!ret) {
		*i2c = chip;
	}

	return ret;
#else
	debug("isl9305_probe: CONFIG_DM_I2C not enabled\n");

	return -ENODEV;
#endif
}

static inline void isl9305_write(struct udevice *dev, uint8_t reg, uint8_t val)
{
#ifdef CONFIG_DM_I2C
	dm_i2c_reg_write(dev, reg, val);
#endif
}

int power_init_board(void)
{
	struct udevice *i2c;

	if (isl9305_probe(&i2c))
		panic("power_init_board: Failed probing ISL9305 PMIC\n");

	debug("power_init_board: Programming ISL9305 PMIC\n");

	/* R.T. ISL9305H I2C programming */

	/* 1. DCD1 set to 1.1V (Epiphany VDD_DSP) */
	isl9305_write(i2c, 0x0, 0x0b);

	/* 2. DCD2 set to 1.35V (DDR3L) */
	isl9305_write(i2c, 0x1, 0x15);

	/* 3. LD01 set to 3.3V (PEC_POWER) */
	isl9305_write(i2c, 0x2, 0x30);

	/* 4. LD02 set to 2.5V (HDMI_GPIO) */
	isl9305_write(i2c, 0x3, 0x20);

	/* 5. Enable the change by writing to SYS_PARAMETER */
	isl9305_write(i2c, 0x5, 0x6f);

	udelay(50000);

	debug("power_init_board: Done\n");

	return 0;
}

int board_late_init(void)
{
	const fpga_desc *desc;
	xilinx_desc *desc_xilinx;

	desc = fpga_get_desc(0);
	if (!desc) {
	       panic("board_late_init: No FPGA found.\n");
	}

	desc_xilinx = desc->devdesc;
	if (!desc_xilinx || desc_xilinx->family != xilinx_zynq ||
	   !desc_xilinx->name) {
	       panic("board_late_init: Unsupported FPGA found.\n");
	}

	env_set("fpga_model", desc_xilinx->name);

	return 0;
}
