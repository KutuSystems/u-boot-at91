/*
 * Control GPIO pins on the fly
 *
 * Copyright (c) 2008-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#define CONFIG_USE_STDINT

#include <common.h>
#include <command.h>
#include <errno.h>
#include <dm.h>
//#include <asm-generic/adc.h>
#include <sama5d2_adc.h>

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
#define BOARD_ADC_VREF (3000000)

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

/** ADC sample data */
static struct _adc_sample _data;

/** ADC test mode */
static struct _adc_test_mode _test_mode;

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


/**
 * \brief Display ADC configuration menu.
 */
static void _display_menu(void)
{
	uint8_t tmp;

	printf("\n\r");
	printf("=========================================================\n\r");
	printf("Menu: press a key to change the configuration.\n\r");
	printf("---------------------------------------------------------\n\r");
	tmp = (_test_mode.trigger_mode == TRIGGER_MODE_SOFTWARE) ? 'X' : ' ';
	printf("[%c] 0: Set ADC trigger mode: Software.\n\r", tmp);
	tmp = (_test_mode.trigger_mode == TRIGGER_MODE_ADTRG) ? 'X' : ' ';
	printf("[%c] 1: Set ADC trigger mode: ADTRG.\n\r", tmp);
	tmp = (_test_mode.trigger_mode == TRIGGER_MODE_TIMER) ? 'X' : ' ';
	printf("[%c] 2: Set ADC trigger mode: Timer TIOA.\n\r", tmp);
	tmp = (_test_mode.trigger_mode == TRIGGER_MODE_ADC_TIMER) ? 'X' : ' ';
	printf("[%c] 3: Set ADC trigger mode: ADC Internal Timer.\n\r", tmp);
	tmp = (_test_mode.sequence_enabled ) ? 'E' : 'D';
	printf("[%c] S: Enable/Disable sequencer.\n\r", tmp);
	tmp = (_test_mode.dma_enabled) ? 'E' : 'D';
	printf("[%c] D: Enable/Disable to tranfer with DMA.\n\r", tmp);
	tmp = (_test_mode.power_save_enabled) ? 'E' : 'D';
	printf("[%c] P: Enable/Disable ADC power save mode.\n\r", tmp);
	printf("=========================================================\n\r");
}


static void _initialize_adc(void)
{
	/* Initialize ADC */
//	adc_initialize();
//	adc_set_ts_mode(0);
	/*
	 * Formula: ADCClock = MCK / ((PRESCAL+1) * 2)
	 * For example, MCK = 64MHZ, PRESCAL = 4, then:
	 *     ADCClock = 64 / ((4+1) * 2) = 6.4MHz;
	 */
	/* Set ADC clock */
//	adc_set_clock(ADC_FREQ);

	/* Formula:
	 *     Startup  Time = startup value / ADCClock
	 *     Transfer Time = (TRANSFER * 2 + 3) / ADCClock
	 *     Tracking Time = (TRACKTIM + 1) / ADCClock
	 *     Settling Time = settling value / ADCClock
	 * For example, ADC clock = 6MHz (166.7 ns)
	 *     Startup time = 512 / 6MHz = 85.3 us
	 *     Transfer Time = (1 * 2 + 3) / 6MHz = 833.3 ns
	 *     Tracking Time = (0 + 1) / 6MHz = 166.7 ns
	 *     Settling Time = 3 / 6MHz = 500 ns
	 */
	/* Set ADC timing */
//	adc_set_timing(ADC_MR_STARTUP_SUT512, 0, 0);
	/* Enable channel number tag */
//	adc_set_tag_enable(true);
}

/**
 * \brief (Re)Sart ADC sample.
 * Initialize ADC, set clock and timing, set ADC to given mode.
 */
static void _configure_adc(void)
{
	uint8_t i = 0;

//	aic_disable(ID_ADC);

	//led_clear(LED_RED);
	//led_clear(LED_BLUE);

	/* Init channel number and reset value */
	for (i = 0; i < NUM_CHANNELS; i++) {
		_data.channel[i] = adc_channel_used[i];
		_data.value[i] = 0;
	}

	/* Enable channels, gain, single mode */
/*	for (i = 0; i < NUM_CHANNELS; i++) {
		adc_enable_channel(_data.channel[i]);
#ifdef CONFIG_HAVE_ADC_INPUT_OFFSET
		adc_disable_channel_differential_input(_data.channel[i]);
#endif
	}
*/

	/* Enable Data ready interrupt */
	uint32_t ier_mask = 0;

	for (i = 0; i < NUM_CHANNELS; i++) {
		ier_mask |= 0x1u << _data.channel[i];
	}
//	adc_enable_it(ier_mask) ;

	/* Set ADC irq handler */
//	aic_set_source_vector(ID_ADC, adc_irq_handler);

	/* Configure trigger mode and start convention */
	switch (_test_mode.trigger_mode) {
		case TRIGGER_MODE_SOFTWARE:
			/* Disable hardware trigger */
//			adc_set_trigger(0);
			/* No trigger, only software trigger can start conversions */
//			adc_set_trigger_mode(ADC_TRGR_TRGMOD_NO_TRIGGER);
			break;
#if def
	    case TRIGGER_MODE_ADTRG:
			pio_clear(&pin_trig);
			adc_set_trigger(ADC_MR_TRGSEL_ADC_TRIG0);
			adc_set_trigger_mode(ADC_TRGR_TRGMOD_EXT_TRIG_ANY);
			break;

		case TRIGGER_MODE_TIMER :
			/* Trigger timer*/
			/* Configure Timer TC0 */
			_configure_tc_trigger();
			adc_set_trigger(ADC_MR_TRGSEL_ADC_TRIG1);
			adc_set_trigger_mode(ADC_TRGR_TRGMOD_EXT_TRIG_RISE);
			break;

		case TRIGGER_MODE_ADC_TIMER :
			/* Trigger timer*/
			adc_set_trigger_mode(ADC_TRGR_TRGMOD_PERIOD_TRIG);
			adc_set_trigger_period(250);
			break;
#endif
	    default :
			break;
	}
   // aic_enable(ID_ADC);
}

/**
 * \brief (Re)init config ADC.
 *
 */
static void _set_adc_configuration(void)
{
//	tc_stop(TC0, 0);
	_configure_adc();
//	adc_start_conversion();
	modif_config = false;
}

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

	if (argc < 2)
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
	_initialize_adc();
	/* Init ADC config */
	_set_adc_configuration();

	_display_menu();

   /* ADC software trigger per 100ms */
   if (_test_mode.trigger_mode == TRIGGER_MODE_SOFTWARE) {
      if (!_data.done) {
         // adc_start_conversion();
         printf("\n\nStarting ADC conversion\n\n");

         _data.done = 1;
         //timer_start_timeout(&timeout, 500);
         //led_toggle(LED_RED);
      }

   }

   printf("NUM_CHANNELS: %d\n ", NUM_CHANNELS);


   printf("Call adc_initialize\n");
   adc_initialize();
   printf("adc_initialize complete\n");


//   return 0;


   /* Check if ADC sample is done */
   if (_data.done) {
      printf("Count: %d \n", count++);
      for (i = 0; i < NUM_CHANNELS; ++i) {
         printf(" CH%02d: %04d mV \n",
               (int)(_data.channel[i]),
               (int)(_data.value[i] * BOARD_ADC_VREF / DIGITAL_MAX));
      }
      printf("\n");
      _data.done = 0;
   }

   return 0;

   /* After console_handler, set configuration adc */
   if (modif_config) {
      _set_adc_configuration();
   }

   return 0;
}

U_BOOT_CMD(adc_test, 4, 0, do_adc_test,
	   "query and control adc pins",
	   "<input|set|clear|toggle> <pin>\n"
	   "    - input/set/clear/toggle the specified pin\n"
	   "adc test status [-a] [<bank> | <pin>]  - show [all/claimed] GPIOs");
