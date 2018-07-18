/*
 * Copyright (C) 2018 Universal Biosensors
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <fdtdec.h>
#include <backlight.h>

#define MAX_MSECS    10000

DECLARE_GLOBAL_DATA_PTR;

/* Initilaise backlight subsystem */
static int do_init(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;

	ret = backlight_init(gd->fdt_blob);
	if (ret) {
		printf("Initialise Audio driver failed\n");
		return CMD_RET_FAILURE;
	}

	return 0;
}

/* play backlight from buffer */
static int do_play(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	int level = 31;

   printf("\n\nBacklight test\n\n");

   if (argc > 1) {
      freq = simple_strtoul(argv[1], NULL, 10);
      if (freq < 200)
         return CMD_RET_USAGE;

      printf("set frequency at %dhz\n",freq);

   }

	if (argc > 2) {
      atten = simple_strtoul(argv[2], NULL, 10);

      if (atten > 127)
         return CMD_RET_USAGE;

       printf("set attenuation at %ddb\n",atten);

   }

   if (argc > 3) {
      msec = simple_strtoul(argv[3], NULL, 10);

      if (msec > MAX_MSECS) {
         msec = MAX_MSECS;
         printf("reduced play to 10 seconds max\n");
      }
      printf("set play length to %d msecs\n",msec);

   }

   // play out backlight
	ret = backlight_play(msec, freq, atten);

	if (ret) {
		printf("play failed\n");
		return CMD_RET_FAILURE;
	}

	printf("audio play complete\n");

	return 0;
}

static cmd_tbl_t cmd_backlight_sub[] = {
	U_BOOT_CMD_MKENT(init, 0, 1, do_init, "", ""),
	U_BOOT_CMD_MKENT(play, 4, 1, do_play, "", ""),
};

/* process backlight command */
static int do_backlight(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 1)
		return CMD_RET_USAGE;

	/* Strip off leading 'backlight' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_backlight_sub[0], ARRAY_SIZE(cmd_backlight_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(backlight, 4, 0, do_backlight,
	"backlight sub-system",
	"set [level] - set backlight to value 0 to 32\n"
);
