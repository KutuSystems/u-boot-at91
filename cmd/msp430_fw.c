/*
 * Copyright (C) 2012 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <stdlib.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <command.h>
#include <fdtdec.h>
#include <asm/arch/atmel_pio4.h>
#include <msp430_fw.h>


#include "component_msp430.h"
#include "atmel_serial.h"

int32_t msp430_initialised = 0;

void msp430_configure(void);


DECLARE_GLOBAL_DATA_PTR;

/*
 * Initialise msp430 interface.
 * put msp430 in reset, set test line low
 * set UART to 9600 baud.
 */
static int do_init(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
   msp430_configure();

	return 0;
}

/* write msp430 code from header msp430_fw.h */
static int do_update(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
   printf("\n\nMSP430 firmware update\n\n");

   if (msp430_initialised == 0)
      msp430_configure();


	printf("MSP430 firmware update complete\n");

	return 0;
}

static int do_msp430_reset(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{

   // set AFE test line low
	atmel_pio4_set_pio_output(AT91_PIO_PORTD, 9, 0);
   // delay for 10 ms
   udelay(10000);

   // set AFE reset line low
	atmel_pio4_set_pio_output(AT91_PIO_PORTD, 12, 0);

   // delay for 10 ms
   udelay(10000);

   // release reset line
	atmel_pio4_set_pio_output(AT91_PIO_PORTD, 12, 1);

   printf("Reset MSP430 succeeded\n");

	return 0;
}

static cmd_tbl_t cmd_msp430_sub[] = {
	U_BOOT_CMD_MKENT(init, 0, 1, do_init, "", ""),
	U_BOOT_CMD_MKENT(update, 4, 1, do_update, "", ""),
	U_BOOT_CMD_MKENT(reset, 0, 1, do_msp430_reset, "", ""),
};


/* process msp430 command */
static int do_msp430(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 1)
		return CMD_RET_USAGE;

	/* Strip off leading 'msp430' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_msp430_sub[0], ARRAY_SIZE(cmd_msp430_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

void msp430_configure(void)
{
   Atmel_flexcom *flexcom4_base =  (Atmel_flexcom *)FLEXCOM4_BASE;
   Atmel_Usart *flexcom4_us = (Atmel_Usart *)FLEXCOM4_US_BASE;

	// enable peripheral clock for usart on FLEXCOM4
   at91_periph_clk_enable(ATMEL_ID_FLEXCOM4);

   // set flexcom4 to uart mode
	writel(0, &flexcom4_base->FLEX_MR);
	writel(0, &flexcom4_us->US_MR);

   msp430_initialised = 1;

	return;
}
U_BOOT_CMD(
	msp430, 5, 0, do_msp430,
	"msp430 sub-system",
	"init - initialise the MSP430 driver\n"
	"update - update the MSP430 firmware\n"
   "reset - reset the msp430 for normal operation\n"
);
