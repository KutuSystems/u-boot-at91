/*
 * Copyright (C) 2012 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <fdtdec.h>
#include <sound.h>

DECLARE_GLOBAL_DATA_PTR;

/* Initilaise sound subsystem */
static int do_init(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;

//	ret = sound_init(gd->fdt_blob);
	if (ret) {
		printf("Initialise Audio driver failed\n");
		return CMD_RET_FAILURE;
	}

	return 0;
}

/* play sound from buffer */
static int do_play(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	int msec = 1000;
	int freq = 400;
	int atten = 0;

   printf("\n\nAudio hardware test\n\n");

   if (argc > 1) {
      freq = simple_strtoul(argv[1], NULL, 10);
      if ((freq > 4) && (freq < 200))
         return CMD_RET_USAGE;

      // temporary exit
      if (freq < 200)
         return CMD_RET_USAGE;

      printf("set frequency at %dhz\n",freq);

   }

	if (argc > 2) {
      atten = simple_strtoul(argv[2], NULL, 10);

      if (atten > 40)
         return CMD_RET_USAGE;

       printf("set attenuation at %ddb\n",atten);

   }

   if (argc > 3) {
      msec = simple_strtoul(argv[3], NULL, 10);

      if (msec > 10000) {
         msec = 10000;
         printf("reduced play to 10 seconds max\n");
      }
      printf("set play length to %d msecs\n",msec);

   }

   // play out sound
//	ret = sound_play(msec, freq, attenuation);

	if (ret) {
		printf("play failed\n");
		return CMD_RET_FAILURE;
	}

	printf("audio play complete\n");

	return 0;
}

static cmd_tbl_t cmd_sound_sub[] = {
	U_BOOT_CMD_MKENT(init, 0, 1, do_init, "", ""),
	U_BOOT_CMD_MKENT(play, 4, 1, do_play, "", ""),
};

/* process sound command */
static int do_sound(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 1)
		return CMD_RET_USAGE;

	/* Strip off leading 'sound' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_sound_sub[0], ARRAY_SIZE(cmd_sound_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}

U_BOOT_CMD(
	sound, 5, 0, do_sound,
	"sound sub-system",
	"init - initialise the sound driver\n"
	"play [freq|1|2|3|4] [attenuation in db] [len] - a sound for len ms\n"
   "at freq hz, or one of 4 wave files\n"
);
