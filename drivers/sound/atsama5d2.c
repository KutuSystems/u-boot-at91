/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <atsama5d2_classd.h>
#include <asm/arch/clk.h>

//#include "peripherals/pmc.h"
#include "component_pmc.h"
#include "component_adc.h"

int sound_play(uint32_t msec, uint32_t frequency)
{
	//sandbox_sdl_sound_start(frequency);
	mdelay(msec);
	//sandbox_sdl_sound_stop();

	return 0;
}

int sound_init(const void *blob)
{

	return 0;
}


bool classd_configure(int audio_clk)
{
	uint8_t i;
	uint32_t mr, intpmr;

	for (i = 0; i < ARRAY_SIZE(audio_info); i++) {
		if (audio_info[i].rate == desc->sample_rate) {
			dsp_clk_set  = audio_info[i].dsp_clk;
			frame_set = audio_info[i].sample_rate;
			break;
		}
	}
	if(i == ARRAY_SIZE(audio_info))
		return false;

	if (!_dspclk_configure(dsp_clk_set))
		return false;

	/* enable peripheral clock, disable audio clock for now */
//	pmc_enable_peripheral(ID_CLASSD);
//	pmc_disable_gck(ID_CLASSD);

   at91_periph_clk_enable(ATMEL_ID_ADC);

   /* Choosing between AudioCLK and USB clock */
   if(audio_clk) {
//      pmc_configure_gck(ID_CLASSD, PMC_PCR_GCKCSS_AUDIO_CLK, 0);
      at91_enable_periph_generated_clk(ATMEL_ID_ADC, GCK_CSS_MAIN_CLK, 11);

   } else {
//      pmc_configure_gck(ID_CLASSD, PMC_PCR_GCKCSS_UPLL_CLK, 4);
      at91_enable_periph_generated_clk(ATMEL_ID_ADC, GCK_CSS_MAIN_CLK, 11);
   }

   clk = at91_get_periph_generated_clk(ATMEL_ID_ADC);

   printf("PLLA clock is : %ld Hz\r\n", get_plla_clk_rate());
   printf("MCK clock is : %ld Hz\r\n", get_mck_clk_rate());
   printf("AUDIO clock is : %d Hz\r\n", clk);

   /* perform soft reset */
	CLASSD->CLASSD_CR  = CLASSD_CR_SWRST;
	CLASSD->CLASSD_IDR = CLASSD_IDR_DATRDY;

	/* initial MR/INTPMR values */
   intpmr = CLASSD_INTPMR_FRAME_FRAME_48K | CLASSD_INTPMR_DSPCLKFREQ_12M288;

	/* configure output mode */
	mr = CLASSD_MR_PWMTYP | CLASSD_MR_NON_OVERLAP | CLASSD_MR_NOVRVAL_20NS;

   /* enable left channel */
   mr |=  CLASSD_MR_LEN;

   /* use mono mode to left channel */
   intpmr |= CLASSD_INTPMR_MONOMODE_MONOLEFT | CLASSD_INTPMR_MONO_ENABLED;

   /* almost full scale */
   intpmr |= CLASSD_INTPMR_ATTL(1);

	/* configure left channel (muted, max attn) */
	if (desc->left_enable)
		mr |= CLASSD_MR_LEN;
	mr |= CLASSD_MR_LMUTE;
	intpmr |= CLASSD_INTPMR_ATTL(CLASSD_INTPMR_ATTL_Msk);

	/* configure right channel (muted, max attn)  */
	if (desc->right_enable)
		mr |= CLASSD_MR_REN;
	mr |= CLASSD_MR_RMUTE;
	intpmr |= CLASSD_INTPMR_ATTR(CLASSD_INTPMR_ATTL_Msk);

        /* De-emphasis Filter */
        //if (desc-> de_emphasis)
            intpmr |= CLASSD_INTPMR_DEEMP;

	/* write configuration */
	CLASSD->CLASSD_MR = mr;
	CLASSD->CLASSD_INTPMR = intpmr;

	/* enable audio clock */
	pmc_enable_gck(ID_CLASSD);


	return (CLASSD->CLASSD_INTSR & CLASSD_INTSR_CFGERR) == 0;
}
