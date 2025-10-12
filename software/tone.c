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



/* wt-keyer */
#include "eeprom.h"
#include "timer.h"
#include "tone.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* Current frequency */
static uint16_t eeToneFreq EEMEM;
uint16_t ToneFreq;



/*****************************************************************************/
/*                             Helper functions                              */
/*****************************************************************************/



static void SetCompareRegA(uint16_t Freq)
/* Set the compare register for the side tone */
{
    /* Since the OC1x bits are toggled on each compare match, the timer must
     * run with twice the frequency that is generated as output.
     */
    uint16_t OvrCmp = (uint16_t)(CLOCK_HZ / (Freq * 2U));

    /* Set both compare registers to this value */
    OCR1AH = (uint8_t) (OvrCmp >> 8);
    OCR1AL = (uint8_t) (OvrCmp >> 0);
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupTone(void)
/* Setup I/O for outputting tones */
{
    /* Read the frequency from the eeprom and set it */
    SetToneFreq(EepromReadWord(&eeToneFreq, TONEFREQ_DEFAULT));

    /* Setup timer 1 */
    TCCR1A = (0 << COM1A1) | (0 << COM1A0) |    /* No output on OC1A */
             (0 << COM1B1) | (0 << COM1B0) |    /* No output on OC1B */
             (0 << FOC1A) | (0 << FOC1B) |
             (0 << WGM11) | (0 << WGM10);       /* CTC mode */
    TCCR1B = (0 << ICNC1) | (0 << ICES1) |      /* No input capture */
             (0 << WGM13) | (1 << WGM12) |      /* CTC mode */
             (0 << CS12) | (0 << CS11) | (1 << CS10);   /* No prescaling */
}



void SaveTone(void)
/* Save all tone settings in the eeprom */
{
    EepromWriteWord(&eeToneFreq, ToneFreq);
}



void SetToneFreq(uint16_t Freq)
/* Set the tone frequency. The frequency is valid for both, the sidetone and
 * the tx tone.
 */
{
    /* Remember the new frequency */
    ToneFreq = Freq;

    /* Since the OC1x bits are toggled on each compare match, the timer must
     * run with twice the frequency that is generated as output.
     */
    uint16_t OvrCmp = (uint16_t)(CLOCK_HZ / (Freq * 2U));

    /* Set both compare registers to this value */
    OCR1AH = (uint8_t) (OvrCmp >> 8);
    OCR1AL = (uint8_t) (OvrCmp >> 0);
    OCR1BH = (uint8_t) (OvrCmp >> 8);
    OCR1BL = (uint8_t) (OvrCmp >> 0);
}



void PlayTone(uint16_t Freq, uint16_t Ticks)
/* Output a tone with the given frequency and duration */
{
    SetCompareRegA(Freq);
    SideToneStart();
    Sleep(Ticks);
    SideToneDone();
    SetCompareRegA(ToneFreq);
}



void SuccessTone(void)
/* Output a short tone that signals a successful operation. This will be
 * always output via the sidetone and will not influence the current tone
 * frequency.
 */
{
    PlayTone(TONEFREQ_SUCCESS, MSEC(70, 0));
}



void FailureTone(void)
/* Output a short tone that signals a failed operation. This will be always
 * output via the sidetone and will not influence the current tone frequency.
 */
{
    PlayTone(TONEFREQ_FAILURE, MSEC(100, 0));
}



