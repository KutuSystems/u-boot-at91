/*
 * Control GPIO pins on the fly
 *
 * Copyright (c) 2008-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#define CONFIG_USE_STDINT

#include <common.h>
#include <asm/io.h>
#include <command.h>
#include <errno.h>
#include <dm.h>
//#include <asm-generic/adc.h>
#include <sama5d2_adc.h>
#include <asm/arch/clk.h>

/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/

/** ADC clock */
#define ADC_FREQ (300000)

/** ADC slected channels */
static uint8_t adc_channel_used[] =
{
	0,
	1,
	2,
	3,
   4,
   5,
   6,
   7,
};

/** Total number of ADC channels in use */
#define NUM_CHANNELS    ARRAY_SIZE(adc_channel_used)

/** ADC convention done mask */
#define ADC_DONE_MASK   ((1<<NUM_CHANNELS) - 1 )

/** ADC VREF */
#define BOARD_ADC_VREF (2970) // should be 3000, but 1% drop on output ???

/** MAXIMUM DIGITAL VALUE */
#define DIGITAL_MAX    4095

#define at91_adc_readl(reg)		   readl(AT91_ADC_BASE + reg)
#define at91_adc_writel(reg, val)	writel(val, AT91_ADC_BASE + reg)

/*----------------------------------------------------------------------------
 *        Local types
 *----------------------------------------------------------------------------*/

/** ADC test mode structure */
struct _adc_test_mode
{
	uint8_t trigger_mode;
	uint8_t dma_enabled;
	uint8_t sequence_enabled;
	uint8_t power_save_enabled;
};

/** ADC trigger modes */
enum _trg_mode
{
	TRIGGER_MODE_SOFTWARE = 0,
	TRIGGER_MODE_ADTRG,
	TRIGGER_MODE_TIMER,
	TRIGGER_MODE_ADC_TIMER,
};

/* A few flags used by show_gpio() */
enum {
	FLAG_SHOW_ALL		= 1 << 0,
	FLAG_SHOW_CHAN		= 1 << 1,
	FLAG_SHOW_NEWLINE	= 1 << 2,
};


/** ADC sample data */
struct _adc_sample
{
	uint8_t channel[NUM_CHANNELS];
	int16_t value[NUM_CHANNELS];
	uint16_t done;
};

bool modif_config = false;
unsigned count = 0;

/*----------------------------------------------------------------------------
 *        Local variables
 *----------------------------------------------------------------------------*/


static int get_function(struct udevice *dev, int offset, bool skip_unused,
		 const char **namep)
{
  	return 0;
}

static int adc_get_function(struct udevice *dev, int offset, const char **namep)
{
	return get_function(dev, offset, true, namep);
}

/*
int adc_get_status(struct udevice *dev, int offset, char *buf, int buffsize)
{

	return 0;
}
*/



static void adc_test_get_description(struct udevice *dev, const char *bank_name,
				 int offset, int *flagsp)
{
	char buf[80];
	int ret;

	ret = adc_get_function(dev, offset, NULL);
	if (ret < 0)
		goto err;
	if (!(*flagsp & FLAG_SHOW_ALL))
		return;
	if ((*flagsp & FLAG_SHOW_CHAN) && bank_name) {
		if (*flagsp & FLAG_SHOW_NEWLINE) {
			putc('\n');
			*flagsp &= ~FLAG_SHOW_NEWLINE;
		}
		printf("Bank %s:\n", bank_name);
		*flagsp &= ~FLAG_SHOW_CHAN;
	}

//	ret = adc_get_status(dev, offset, buf, sizeof(buf));
//	if (ret)
//		goto err;

	printf("%s\n", buf);
	return;
err:
	printf("Error %d\n", ret);
}

static int do_adc_test_status(bool all, const char *gpio_name)
{
	struct udevice *dev;
	int flags;
	int ret;

	flags = 0;
	if (gpio_name && !*gpio_name)
		gpio_name = NULL;
	for (ret = uclass_first_device(UCLASS_GPIO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {

		flags |= FLAG_SHOW_CHAN;
		if (all)
			flags |= FLAG_SHOW_ALL;

		if (!gpio_name || !strncmp(gpio_name, 0, 0)) {
			const char *p = NULL;
			int offset;

			p = gpio_name;
			if (gpio_name && *p) {
				offset = simple_strtoul(p, NULL, 10);
				adc_test_get_description(dev, 0, offset,
						     &flags);
			} else {
				for (offset = 0; offset < 4; offset++) {
					adc_test_get_description(dev, 0,
							     offset, &flags);
				}
			}
		}
		/* Add a newline between channel names */
		if (!(flags & FLAG_SHOW_CHAN))
			flags |= FLAG_SHOW_NEWLINE;
	}

	return ret;
}

/**
 *  \brief adc_adc12 Application entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
static int do_adc_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
   uint8_t i;
   const char *str_cmd, *str_adc_test = NULL;
   bool all = false;

   /** ADC sample data */
   // static struct _adc_sample data;

   /** ADC test mode */
   static struct _adc_test_mode test_mode;


   /* Configure console interrupts */
   //	console_set_rx_handler(console_handler);
   //	console_enable_rx_interrupt();

   /* Output example information */
   printf("\n\nADC12 hardware test\n\n");

   if (argc > 2)
      return CMD_RET_USAGE;
   str_cmd = argv[1];
   argc -= 2;
   argv += 2;
   if (argc > 0 && !strcmp(*argv, "-a")) {
      all = true;
      argc--;
      argv++;
   }

   if (argc > 0)
      str_adc_test = *argv;
   if (!strncmp(str_cmd, "status", 2)) {
      return cmd_process_error(cmdtp, do_adc_test_status(all, str_adc_test));
   }

   /* Configure trigger pins
      pio_configure(&pin_trig, 1);
      pio_clear(&pin_trig);
      pio_configure(pin_adtrg, ARRAY_SIZE(pin_adtrg));*/

   /* Set defaut ADC test mode */
   memset((void *)&test_mode, 0, sizeof(test_mode));
   test_mode.trigger_mode = TRIGGER_MODE_SOFTWARE;
   test_mode.dma_enabled = 0;
   test_mode.sequence_enabled = 0;

   /* Initialize ADC clock */

   printf("Initialize ADC\n");
   adc_initialize();

   adc_ts_calibration();

   /* Init ADC config */
   //adc_set_trigger(0);
   /* No trigger, only software trigger can start conversions */
   adc_set_trigger_mode(0);

  // adc_set_sequence_mode(0); // normal sequence


   printf("ADC Clock Frequency is %dHz\n",at91_get_periph_generated_clk(ATMEL_ID_ADC));

   adc_start_conversion();

   printf("\n\nStarting ADC conversion\n\n");

   printf("NUM_CHANNELS: %d\n ", adc_get_num_channels());

   /* Check if ADC sample is done */
   printf("Count: %d \n", count++);
   for (i = 0; i < NUM_CHANNELS; ++i) {
      printf(" CH%02d: %04d counts %04d mV \n",i,
            (int)(adc_get_converted_data(i)),(int)((adc_get_converted_data(i) * BOARD_ADC_VREF)/DIGITAL_MAX));
   }
   printf("\n");

   // output values in voltage
   printf(" CH00: +1.2V rail  = %04dmV \n",((adc_get_converted_data(0) * BOARD_ADC_VREF)/ DIGITAL_MAX));
   printf(" CH01: +1.8V rail  = %04dmV \n",((adc_get_converted_data(1) * BOARD_ADC_VREF)/ DIGITAL_MAX));
   printf(" CH02: +3.3V rail  = %04dmV \n",((adc_get_converted_data(2) * BOARD_ADC_VREF * 2)/ DIGITAL_MAX));
   printf(" CH03: +5.0V rail  = %04dmV \n",((adc_get_converted_data(3) * BOARD_ADC_VREF * 2)/ DIGITAL_MAX));
   printf(" CH04: +3.0VA rail = %04dmV \n",((adc_get_converted_data(4) * BOARD_ADC_VREF * 2)/ DIGITAL_MAX));
   printf(" CH05: +5Vusb rail = %04dmV \n",((adc_get_converted_data(5) * BOARD_ADC_VREF * 2)/ DIGITAL_MAX));
   printf(" CH06: +SMPS rail  = %04dmV \n",((adc_get_converted_data(6) * BOARD_ADC_VREF * 2)/ DIGITAL_MAX));
   printf(" CH07: +LIP0 rail  = %04dmV \n",((adc_get_converted_data(7) * BOARD_ADC_VREF * 2)/ DIGITAL_MAX));

   printf("\n");


   return 0;
}

U_BOOT_CMD(adc_test, 4, 0, do_adc_test,
	   "read adc channels",
	   "<read|status> \n");
