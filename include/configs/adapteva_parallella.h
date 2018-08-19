/*
 * Copyright (C) 2013-2015 Adapteva, Inc.
 *
 * Authors:
 *	Roman Trogan <support@adapteva.com>
 *	Ola Jeppsson <ola@adapteva.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ADAPTEVA_PARALLELLA_H
#define __CONFIG_ADAPTEVA_PARALLELLA_H

#define CONFIG_ZYNQ_SDHCI1

#define CONFIG_ZYNQ_I2C0
#define CONFIG_ZYNQ_I2C1

#define CONFIG_ENV_OFFSET		0x004E0000

#define CONFIG_PREBOOT
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"ethaddr=04:4f:8b:00:00:00\0"   \
	"ipaddr=10.11.12.13\0" \
	"serverip=10.11.12.1\0" \
	"bootenv=uEnv.txt\0" \
	"fdt_high=0x20000000\0"				\
	"kernel_image=uImage\0" \
	"old_bitstream_image=parallella.bit.bin\0" \
	"old_devicetree_image=devicetree.dtb\0" \
	"loadbootenv_addr=0x2000000\0" \
	"loadbootenv=load mmc 0 ${loadbootenv_addr} ${bootenv}\0" \
	"slowblink=" \
		"led 0 toggle && sleep 1 && led 0 toggle && sleep 1 || true\0" \
	"importbootenv=echo Importing environment from SD ...; " \
		"env import -t ${loadbootenv_addr} $filesize\0" \
	"importbootenvtftp=echo Importing environment from TFTP ...; " \
		"env import -t ${loadbootenv_addr} $filesize\0" \
	"preboot=if mmcinfo; then " \
			"led 0 on; " \
			"run slowblink; " \
			"env set modeboot sdboot_old; " \
			"run loadbootenv && " \
			"run importbootenv && " \
			"env set modeboot sdboot; " \
		"else " \
			"led 0 off; " \
			"env set modeboot tftpboot; " \
		"fi; \0" \
	"sdboot_old=" \
		"echo Legacy boot from SD card...; " \
		"load mmc 0 0x4000000 ${old_bitstream_image} && " \
		"fpga load 0 0x4000000 $filesize && " \
		"run slowblink; " \
		"load mmc 0 0x3000000 ${kernel_image} && " \
		"load mmc 0 0x2A00000 ${old_devicetree_image} && " \
		"run slowblink && " \
		"bootm 0x3000000 - 0x2A00000; " \
		"led 0 off\0" \
	"tftpboot=while true; do " \
			"echo TFTPing second stage boot script... && " \
			"tftpboot ${loadbootenv_addr} ${bootenv} && " \
			"run importbootenvtftp && " \
			"run tftpboot_stage2; " \
		"done\0"


#include <configs/zynq-common.h>

#endif /* __CONFIG_ADAPTEVA_PARALLELLA_H */
