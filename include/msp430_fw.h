/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar < rcsekar@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MSP430_FW_H__
#define __MSP430_FW_H__


/*
 * Generates square wave sound data for 1 second
 *
 * @param data          data buffer pointer
 * @param size          size of the buffer
 * @param freq          frequency of the wave
 */
//void sound_create_square_wave(unsigned short *data, int size, uint32_t freq);

/*
 * Initialises audio sub system
 * @param blob	Pointer of device tree node or NULL if none.
 * @return	int value 0 for success, -1 for error
 */
//int sound_init(const void *blob);

/*
 * plays the pcm data buffer in pcm_data.h through i2s1 to make the
 * sine wave sound
 *
 * @return	int 0 for success, -1 for error
 */
//int sound_play(uint32_t msec, uint32_t frequency, uint32_t attenuation);

#endif  /* __MSP430_FW_H__ */
