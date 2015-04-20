/*
 * Copyright (C) 2015 Adapteva, Inc.
 *
 * Author: Ola Jeppsson <ola@adapteva.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#define pr_fmt(fmt) "adapteva-parallella: " fmt

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <fpga.h>
#include <xilinx.h>

DECLARE_GLOBAL_DATA_PTR;

#define ISL9305_I2C_ADDR 0x68 /* PMIC */

void green_led_on(void)
{
	writel(0xff7f0080, 0xe000a000);
}

void green_led_off(void)
{
	/* Clear mask/data to make GPIO[7] LOW */
	writel(0xff7f0000, 0xe000a000);
}

void __led_toggle(led_id_t mask)
{
	bool toggled = readl(0xe000a040) & 0x00000080;

	if (toggled)
		green_led_off();
	else
		green_led_on();
}

void __led_set(led_id_t mask, int state)
{
	if (state == STATUS_LED_OFF)
		green_led_off();
	else
		green_led_on();
}

void __led_init(led_id_t mask, int state)
{
	/* 1. Turn GPIO[7] to output
	 * 2. Enable GPIO[7] */
	writel(0x00000080, 0xe000a204);
	writel(0x00000080, 0xe000a208);

	__led_set(mask, state);
}

static inline void led_blink(led_id_t mask, int n)
{
	for  (n = n * 2; n > 0; n--) {
		udelay(75000);
		__led_toggle(mask);
	}
}

static inline void green_led_blink(int n)
{
	led_blink(STATUS_LED_BIT, n);
}

static inline int isl9305_probe(void)
{
	return i2c_probe(ISL9305_I2C_ADDR);
}

static inline void isl9305_write(uint8_t reg, uint8_t val)
{
	i2c_reg_write(ISL9305_I2C_ADDR, reg, val);
}

int power_init_board(void)
{
	if (isl9305_probe())
		panic("power_init_board: Failed probing ISL9305 PMIC\n");

	debug("power_init_board: Programming ISL9305 PMIC\n");

	/* R.T. ISL9305H I2C programming */

	/* 1. DCD1 set to 1.1V (Epiphany VDD_DSP) */
	isl9305_write(0x0, 0x0b);

	/* 2. DCD2 set to 1.35V (DDR3L) */
	isl9305_write(0x1, 0x15);

	/* 3. LD01 set to 3.3V (PEC_POWER) */
	isl9305_write(0x2, 0x30);

	/* 4. LD02 set to 2.5V (HDMI_GPIO) */
	isl9305_write(0x3, 0x20);

	/* 5. Enable the change by writing to SYS_PARAMETER */
	isl9305_write(0x5, 0x6f);

	udelay(50000);

	debug("power_init_board: Done\n");

	return 0;
}

int board_late_init(void)
{
	const fpga_desc *desc;
	xilinx_desc *desc_xilinx;

	green_led_blink(1);

	desc = fpga_get_desc(0);
	if (!desc) {
		green_led_off();
		panic("board_late_init: No FPGA found.\n");
	}

	desc_xilinx = desc->devdesc;
	if (!desc_xilinx || desc_xilinx->family != xilinx_zynq ||
	    !desc_xilinx->name) {
		green_led_off();
		panic("board_late_init: Unsupported FPGA found.\n");
	}

	green_led_blink(4);

	setenv("fpga_model", desc_xilinx->name);

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board:\tAdapteva Parallella\n");
	return 0;
}
#endif

