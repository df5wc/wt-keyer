/*****************************************************************************/
/*                                                                           */
/*                                  tone.h                                   */
/*                                                                           */
/*                              Tone generation                              */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/* (C) 2025,      Ullrich von Bassewitz                                      */
/*                Roemerstrasse 52                                           */
/*                D-70794 Filderstadt                                        */
/* EMail:         uz@df5wc.org                                               */
/*                                                                           */
/*                                                                           */
/* This software is provided 'as-is', without any expressed or implied       */
/* warranty.  In no event will the authors be held liable for any damages    */
/* arising from the use of this software.                                    */
/*                                                                           */
/* Permission is granted to anyone to use this software for any purpose,     */
/* including commercial applications, and to alter it and redistribute it    */
/* freely, subject to the following restrictions:                            */
/*                                                                           */
/* 1. The origin of this software must not be misrepresented; you must not   */
/*    claim that you wrote the original software. If you use this software   */
/*    in a product, an acknowledgment in the product documentation would be  */
/*    appreciated but is not required.                                       */
/* 2. Altered source versions must be plainly marked as such, and must not   */
/*    be misrepresented as being the original software.                      */
/* 3. This notice may not be removed or altered from any source              */
/*    distribution.                                                          */
/*                                                                           */
/*****************************************************************************/



#ifndef WTKEYER_TONE_H
#define WTKEYER_TONE_H



#include <stdint.h>
#include <avr/io.h>



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



#define TONEFREQ_MIN            500
#define TONEFREQ_MAX            800
#define TONEFREQ_DEFAULT        600
#define TONEFREQ_FAILURE        400
#define TONEFREQ_SUCCESS       1200

uint16_t ToneFreq;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupTone(void);
/* Setup I/O for outputting tones */

void SaveTone(void);
/* Save all tone settings in the eeprom */

void SetToneFreq(uint16_t Freq);
/* Set the tone frequency. The frequency is valid for both, the sidetone and
 * the tx tone.
 */

void PlayTone(uint16_t Freq, uint16_t Ticks);
/* Output a tone with the given frequency and duration */

void SuccessTone(void);
/* Output a short tone that signals a successful operation. This will be
 * always output via the sidetone and will not influence the current tone
 * frequency.
 */

void FailureTone(void);
/* Output a short tone that signals a failed operation. This will be always
 * output via the sidetone and will not influence the current tone frequency.
 */

static inline void SideToneStart(void)
/* Start sidetone output */
{
    /* Set COM1A1:COM1A0 to 01 */
    TCCR1A = (TCCR1A & ~(1 << COM1A1)) | (1 << COM1A0);
}

static inline void SideToneDone(void)
/* End sidetone output */
{
    /* Set COM1A1:COM1A0 to 00 */
    TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0));
}

static inline void TxToneStart(void)
/* Start TX tone output */
{
    /* Set COM1B1:COM1B0 to 01 */
    TCCR1A = (TCCR1A & ~(1 << COM1B1)) | (1 << COM1B0);
}

static inline void TxToneDone(void)
/* End TX tone output */
{
    /* Set COM1B1:COM1B0 to 00 */
    TCCR1A &= ~((1 << COM1B1) | (1 << COM1B0));
}



/* End of tone.h */
#endif




