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

bool classd_configure(int audio_clk);


int32_t isin_S4(int32_t x)
{
    int32_t c, y;
    static const int qN= 13, qA= 12, B=19900, C=3516;

    c= x<<(30-qN);              // Semi-circle info into carry.
    x -= 1<<qN;                 // sine -> cosine calc

    x= x<<(31-qN);              // Mask with PI
    x= x>>(31-qN);              // Note: SIGNED shift! (to qN)
    x= x*x>>(2*qN-14);          // x=x^2 To Q14

    y= B - (x*C>>14);           // B - x^2*C
    y= (1<<qA)-(x*y>>16);       // A - x^2*(B-x^2*C)

    return c>=0 ? y : -y;
}


void Custom_Sine_Function_PlayOnce(uint32_t msec, uint32_t frequency)
{
   uint32_t i = 0;
   int16_t buffer = 0;
   int16_t value = 0;
   int32_t step = 0;
   int32_t audio = 0;
   int32_t length = msec*48;

   // unmute output
   //_playback_start();

   step = (32768 * frequency)/48000;

   do {
      // last_attn_level = current_attn_level;

      if (CLASSD->CLASSD_ISR & CLASSD_ISR_DATRDY) {
         CLASSD->CLASSD_THR = audio;

         buffer += step;

         if (buffer > 65536)
            buffer -= 65536;

         value = (isin_S4(buffer) * 32729)/4096;

         audio = value << 16;
         audio += value;
         i++;

         //printf("Sin Value of %d = %d\r\n",buffer,value);
      }
   } while (i < length);

   // set output to 0
   CLASSD->CLASSD_THR = 0;

   printf("index %d\n\r",i);

   // mute output
   //_playback_stop();
}

int sound_play(uint32_t msec, uint32_t frequency, uint32_t attenuation)
{
	//sandbox_sdl_sound_start(frequency);
	Custom_Sine_Function_PlayOnce(msec,frequency);
	//sandbox_sdl_sound_stop();

	return 0;
}

int sound_init(const void *blob)
{

   classd_configure(0);
	return 0;
}


bool classd_configure(int audio_clk)
{
	uint32_t mr, intpmr, clk;

	/* enable peripheral clock, disable audio clock for now */

   at91_periph_clk_enable(ATMEL_ID_CLASSD);

   /* Choosing between AudioCLK and USB clock */
   if(audio_clk) {
      at91_enable_periph_generated_clk(ATMEL_ID_CLASSD, GCK_CSS_PLLA_CLK, 4);
   } else {
      at91_enable_periph_generated_clk(ATMEL_ID_CLASSD, GCK_CSS_UPLL_CLK, 4);
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
//   mr |= CLASSD_MR_LMUTE;
//   intpmr |= CLASSD_INTPMR_ATTL(CLASSD_INTPMR_ATTL_Msk);

   /* configure right channel (muted, max attn)  */
   mr |= CLASSD_MR_REN;
//   mr |= CLASSD_MR_RMUTE;
//   intpmr |= CLASSD_INTPMR_ATTR(CLASSD_INTPMR_ATTL_Msk);

   /* De-emphasis Filter */
   //if (desc-> de_emphasis)
//   intpmr |= CLASSD_INTPMR_DEEMP;

   /* write configuration */
   CLASSD->CLASSD_MR = mr;
   CLASSD->CLASSD_INTPMR = intpmr;

	return (CLASSD->CLASSD_INTSR & CLASSD_INTSR_CFGERR) == 0;
}
