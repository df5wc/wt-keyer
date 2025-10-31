/*****************************************************************************/
/*                                                                           */
/*                                   cw.c                                    */
/*                                                                           */
/*                         CW definitions and keying                         */
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



#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>

/* wt-keyer */
#include "cw.h"
#include "eeprom.h"
#include "tone.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* PARIS has 50 dits, so ...
 * ... dit length in seconds: 60 / (50 * WPM) = 6 / (5 * WPM)
 * dit length in timer increments: (6 * IRQ_HZ) / (5 * WPM)
 * Since IRQ_HZ is dividable by 5: 6 * IRQ_HZ / 5 / WPM
 */
#define DIT_LENGTH(WPM)         ((uint16_t) (6UL * IRQ_HZ / 5UL / (WPM)))

/* Inputs */
volatile uint8_t Keys;
volatile uint8_t ChangedKeys;

/* Current WPM setting and it's dit and dah lengths */
static uint8_t  eeWpm EEMEM;
uint8_t         Wpm;
uint16_t        ElementTimes[3];

/* Input from straight key? */
bool StraightKey;

/* Paddles swapped? */
static uint8_t  eePaddleSwapped EEMEM;
bool            PaddleSwapped;

/* Translation table: ASCII characters to CW representation. The table is
 * sorted so doing a binary search for mapping char -> CwChar is possible
 * but we will currently search linear since the pointer calculation and
 * function overhead will eat up most of the advantages of a binary search
 * and the routines to map a character back and forth aren't used in time
 * critical situations anyway.
 */
struct TransEntry {
    char        C;
    CwChar      Cw;
};
typedef struct TransEntry TransEntry;
const TransEntry TransTable[] PROGMEM = {
    { '-', CW_DASH      },
    { '/', CW_STROKE    },
    { '0', CW_0         },
    { '1', CW_1         },
    { '2', CW_2         },
    { '3', CW_3         },
    { '4', CW_4         },
    { '5', CW_5         },
    { '6', CW_6         },
    { '7', CW_7         },
    { '8', CW_8         },
    { '9', CW_9         },
    { '?', CW_QM        },
    { 'A', CW_A         },
    { 'B', CW_B         },
    { 'C', CW_C         },
    { 'D', CW_D         },
    { 'E', CW_E         },
    { 'F', CW_F         },
    { 'G', CW_G         },
    { 'H', CW_H         },
    { 'I', CW_I         },
    { 'J', CW_J         },
    { 'K', CW_K         },
    { 'L', CW_L         },
    { 'M', CW_M         },
    { 'N', CW_N         },
    { 'O', CW_O         },
    { 'P', CW_P         },
    { 'Q', CW_Q         },
    { 'R', CW_R         },
    { 'S', CW_S         },
    { 'T', CW_T         },
    { 'U', CW_U         },
    { 'V', CW_V         },
    { 'W', CW_W         },
    { 'X', CW_X         },
    { 'Y', CW_Y         },
    { 'Z', CW_Z         },
    { 'a', CW_A         },
    { 'b', CW_B         },
    { 'c', CW_C         },
    { 'd', CW_D         },
    { 'e', CW_E         },
    { 'f', CW_F         },
    { 'g', CW_G         },
    { 'h', CW_H         },
    { 'i', CW_I         },
    { 'j', CW_J         },
    { 'k', CW_K         },
    { 'l', CW_L         },
    { 'm', CW_M         },
    { 'n', CW_N         },
    { 'o', CW_O         },
    { 'p', CW_P         },
    { 'q', CW_Q         },
    { 'r', CW_R         },
    { 's', CW_S         },
    { 't', CW_T         },
    { 'u', CW_U         },
    { 'v', CW_V         },
    { 'w', CW_W         },
    { 'x', CW_X         },
    { 'y', CW_Y         },
    { 'z', CW_Z         },
};



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupCw(void)
/* Module setup */
{
    /* Read the wpm settings from the eeprom */
    SetCwWpm(EepromReadByte(&eeWpm, WPM_DEFAULT));

    /* Before reading the PaddleSwapped flag, we check if the input is a
     * straight key. We detect this from the dah input being active when
     * the device comes up. A straight key uses a mono instead of a stereo
     * jack which shortens dah to ground.
     */
    Sleep(10);
    if (Dah(Keys)) {
        StraightKey = true;
        PaddleSwapped = false;  /* Set but don't save */
    }

    /* Now read the remaining stuff from the eeprom */
    PaddleSwapped = EepromReadByte(&eePaddleSwapped, false);
}



void SaveCw(void)
/* Save all cw settings in the eeprom */
{
    EepromWriteByte(&eeWpm, Wpm);
    EepromWriteByte(&eePaddleSwapped, PaddleSwapped);
}



void SetCwWpm(uint8_t NewWpm)
/* Change the WPM setting */
{
    Wpm = NewWpm;
    ElementTimes[EL_DIT] = DIT_LENGTH(Wpm);
    ElementTimes[EL_DAH] = ElementTimes[EL_DIT] * 3;
    ElementTimes[EL_PAUSE] = ElementTimes[EL_DIT];
}



char CwToAscii(CwChar C)
/* Convert a CW character to its ASCII counterpart. Returns 0xFF if unknown. */
{
    /* We will always return upper case characters since these appear before
     * their lower case equivalents in the table.
     */
    for (uint8_t I = 0; I < sizeof(TransTable)/sizeof(TransTable[0]); ++I) {
        if (pgm_read_word(&TransTable[I].Cw) == C) {
            return pgm_read_byte(&TransTable[I].C);
        }
    }
    /* Not found */
    return 0xFF;
}



CwChar AsciiToCw(char C)
/* Convert an ASCII character to its CW counterpart. Returns 0 if unknown. */
{
    for (uint8_t I = 0; I < sizeof(TransTable)/sizeof(TransTable[0]); ++I) {
        if (pgm_read_byte(&TransTable[I].C) == C) {
            return pgm_read_word(&TransTable[I].Cw);
        }
    }
    /* Not found */
    return 0x0000;
}



static void PlayDit(void)
/* Output one dit for config purposes */
{
    SideToneStart();
    Sleep(ElementTime(EL_DIT));
    SideToneDone();
    Sleep(ElementTime(EL_PAUSE));
}



static void PlayDah(void)
/* Output one dah for config purposes */
{
    SideToneStart();
    Sleep(ElementTime(EL_DAH));
    SideToneDone();
    Sleep(ElementTime(EL_PAUSE));
}



void PlayCwChar(CwChar C)
/* Play a CW character on the sidetone */
{
    while (C != 0x0000) {
        switch (C >> 14) {
            case EL_DIT:        PlayDit();      break;
            case EL_DAH:        PlayDah();      break;
            default:            ;       /* Ignore the pause */
        }
        C <<= 2;
    }
    /* Make the pause 3 dit lengths total */
    Sleep(2 * ElementTime(EL_PAUSE));
}



static void PlayNumberInt(uint16_t Number, uint8_t Digits)
/* Play a number with the given digits on the sidetone. Internal function. */
{
    if (Digits > 1) {
        PlayNumberInt(Number / 10U, Digits - 1);
    }
    switch ((uint8_t) (Number % 10)) {
        case 0: PlayCwChar(CW_0); break;
        case 1: PlayCwChar(CW_1); break;
        case 2: PlayCwChar(CW_2); break;
        case 3: PlayCwChar(CW_3); break;
        case 4: PlayCwChar(CW_4); break;
        case 5: PlayCwChar(CW_5); break;
        case 6: PlayCwChar(CW_6); break;
        case 7: PlayCwChar(CW_7); break;
        case 8: PlayCwChar(CW_8); break;
        case 9: PlayCwChar(CW_9); break;
    }
}



void PlayNumber(uint16_t Number, uint8_t Digits)
/* Play a number with the given digits on the sidetone */
{
    PlayNumberInt(Number, Digits);
    /* Make the pause 7 dit lengths total */
    AddWordPause();
}



int8_t IsCwDigit(CwChar C)
/* If this is a CW digit, return its value, otherwise return -1 */
{
    switch (C) {
        case CW_0: return 0;
        case CW_1: return 1;
        case CW_2: return 2;
        case CW_3: return 3;
        case CW_4: return 4;
        case CW_5: return 5;
        case CW_6: return 6;
        case CW_7: return 7;
        case CW_8: return 8;
        case CW_9: return 9;
        default:   return -1;
    }
}



