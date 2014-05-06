/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <netdev.h>
#include <zynqpl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
Xilinx_desc fpga;

/* It can be done differently */
Xilinx_desc fpga010 = XILINX_XC7Z010_DESC(0x10);
Xilinx_desc fpga015 = XILINX_XC7Z015_DESC(0x15);
Xilinx_desc fpga020 = XILINX_XC7Z020_DESC(0x20);
Xilinx_desc fpga030 = XILINX_XC7Z030_DESC(0x30);
Xilinx_desc fpga045 = XILINX_XC7Z045_DESC(0x45);
Xilinx_desc fpga100 = XILINX_XC7Z100_DESC(0x100);
#endif

#if 0
extern int i2c_write(unsigned char chip, unsigned int addr, int alen, unsigned char *buffer, int len);

static inline void i2c_reg_write(unsigned char addr, int alen, unsigned char reg, unsigned char val)
{
	int rc;

	rc=i2c_write(addr, reg, alen, &val, 1);

}
#endif

int board_init(void)
{
#ifndef CONFIG_SPL_BUILD
  //A.O: Note! Current voltages only good for Gen0, need to modify code
  //to incorporate Gen1 power changes.

  //R.T. ISL9305H I2C programming
  // 1. DCD1 set to 1.0V (Epiphany)
  // i2cset 0 0x68 0x00 0x07

  i2c_reg_write(CONFIG_SYS_I2C_ADDR, 0x0, 0x07);

  //2. DCD2 set to 1.35V (DDR3L)
  //i2cset 0 0x68 0x01 0x27
	
  i2c_reg_write(CONFIG_SYS_I2C_ADDR, 0x1, 0x27);

  //3. LD01 set to 3.3V (PEC_POWER)
  // i2cset 0 0x68 0x02 0x20

  i2c_reg_write(CONFIG_SYS_I2C_ADDR, 0x2 ,0x20);

  // 4. LD02 set to 2.5V (HDMI_GPIO)
  // i2cset 0 0x68 0x03 0x12

  i2c_reg_write(CONFIG_SYS_I2C_ADDR, 0x3, 0x12);

  // 5. Enable the change by writing to SYS_PARAMETER
  // i2cset 0 0x68 0x05 0x6f

  i2c_reg_write(CONFIG_SYS_I2C_ADDR, 0x5, 0x6f);

  udelay (50000);

#endif 
  //	To turn on LED/un-reset Ethernet PHY:
  //	----------------------------------------------------
  //	Turn GPIO[7] to output
  //	Enable GPIO[7]
  //	Set mask/data to make GPIO[7] HIGH
  
  //A.O.-->No longer needed, power issue fixed, remove code later

  //A.0-->cut
  //XIo_Out32(0xe000a204, 0x80);
  //XIo_Out32(0xe000a208, 0x80);
  //XIo_Out32(0xe000a000, 0xff7f0080);
  
  //A.0-->end cut

#if defined(CONFIG_ENV_IS_IN_EEPROM) && !defined(CONFIG_SPL_BUILD)
	unsigned char eepromsel = CONFIG_SYS_I2C_MUX_EEPROM_SEL;
#endif
#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	u32 idcode;

	idcode = zynq_slcr_get_idcode();

	switch (idcode) {
	case XILINX_ZYNQ_7010:
		fpga = fpga010;
		break;
	case XILINX_ZYNQ_7015:
		fpga = fpga015;
		break;
	case XILINX_ZYNQ_7020:
		fpga = fpga020;
		break;
	case XILINX_ZYNQ_7030:
		fpga = fpga030;
		break;
	case XILINX_ZYNQ_7045:
		fpga = fpga045;
		break;
	case XILINX_ZYNQ_7100:
		fpga = fpga100;
		break;
	}
#endif

	/* temporary hack to clear pending irqs before Linux as it
	 * will hang Linux
	 */
	writel(0x26d, 0xe0001014);

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif
#if defined(CONFIG_ENV_IS_IN_EEPROM) && !defined(CONFIG_SPL_BUILD)
	if (eeprom_write(CONFIG_SYS_I2C_MUX_ADDR, 0, &eepromsel, 1))
		puts("I2C:EEPROM selection failed\n");
#endif
	return 0;
}

int board_late_init(void)
{
	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
	case ZYNQ_BM_QSPI:
		setenv("modeboot", "qspiboot");
		break;
	case ZYNQ_BM_NAND:
		setenv("modeboot", "nandboot");
		break;
	case ZYNQ_BM_NOR:
		setenv("modeboot", "norboot");
		break;
	case ZYNQ_BM_SD:
		setenv("modeboot", "sdboot");
		break;
	case ZYNQ_BM_JTAG:
		setenv("modeboot", "jtagboot");
		break;
	default:
		setenv("modeboot", "");
		break;
	}

	return 0;
}

int board_eth_init(bd_t *bis)
{
	u32 ret = 0;

#ifdef CONFIG_XILINX_AXIEMAC
	ret |= xilinx_axiemac_initialize(bis, XILINX_AXIEMAC_BASEADDR,
						XILINX_AXIDMA_BASEADDR);
#endif
#ifdef CONFIG_XILINX_EMACLITE
	u32 txpp = 0;
	u32 rxpp = 0;
# ifdef CONFIG_XILINX_EMACLITE_TX_PING_PONG
	txpp = 1;
# endif
# ifdef CONFIG_XILINX_EMACLITE_RX_PING_PONG
	rxpp = 1;
# endif
	ret |= xilinx_emaclite_initialize(bis, XILINX_EMACLITE_BASEADDR,
			txpp, rxpp);
#endif

#if defined(CONFIG_ZYNQ_GEM)
# if defined(CONFIG_ZYNQ_GEM0)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR0,
						CONFIG_ZYNQ_GEM_PHY_ADDR0, 0);
# endif
# if defined(CONFIG_ZYNQ_GEM1)
	ret |= zynq_gem_initialize(bis, ZYNQ_GEM_BASEADDR1,
						CONFIG_ZYNQ_GEM_PHY_ADDR1, 0);
# endif
#endif
	return ret;
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	int ret = 0;

#if defined(CONFIG_ZYNQ_SDHCI)
# if defined(CONFIG_ZYNQ_SDHCI0)
	ret = zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR0);
# endif
# if defined(CONFIG_ZYNQ_SDHCI1)
	ret |= zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR1);
# endif
#endif
	return ret;
}
#endif

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	zynq_ddrc_init();

	return 0;
}
