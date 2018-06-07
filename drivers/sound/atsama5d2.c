/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clk.h>

//#include "peripherals/pmc.h"
//#include "component_pmc.h"
#include "component_classd.h"

int sound_play(uint32_t msec, uint32_t frequency, uint32_t attenuation)
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
	uint32_t mr, intpmr, clk;

	/* enable peripheral clock, disable audio clock for now */

   at91_periph_clk_enable(ATMEL_ID_CLASSD);

   /* Choosing between AudioCLK and USB clock */
   if(audio_clk) {
      at91_enable_periph_generated_clk(ATMEL_ID_CLASSD, GCK_CSS_MAIN_CLK, 11);
   } else {
      at91_enable_periph_generated_clk(ATMEL_ID_CLASSD, GCK_CSS_UPLL_CLK, 11);
   }

   clk = at91_get_periph_generated_clk(ATMEL_ID_CLASSD);

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
   mr |= CLASSD_MR_LEN;
   mr |= CLASSD_MR_LMUTE;
   intpmr |= CLASSD_INTPMR_ATTL(CLASSD_INTPMR_ATTL_Msk);

   /* configure right channel (muted, max attn)  */
   mr |= CLASSD_MR_REN;
   mr |= CLASSD_MR_RMUTE;
   intpmr |= CLASSD_INTPMR_ATTR(CLASSD_INTPMR_ATTL_Msk);

   /* De-emphasis Filter */
   //if (desc-> de_emphasis)
   intpmr |= CLASSD_INTPMR_DEEMP;

   /* write configuration */
   CLASSD->CLASSD_MR = mr;
   CLASSD->CLASSD_INTPMR = intpmr;

	return (CLASSD->CLASSD_INTSR & CLASSD_INTSR_CFGERR) == 0;
}
