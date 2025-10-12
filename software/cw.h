/*****************************************************************************/
/*                                                                           */
/*                                   cw.h                                    */
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



#ifndef WTKEYER_CW_H
#define WTKEYER_CW_H



#include <stdbool.h>
#include <stdint.h>

/* wt-keyer */
#include "timer.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* We allow adjustments from 12 WPM to 40 WPM */
#define WPM_MIN                 12
#define WPM_MAX                 40
#define WPM_DEFAULT             20

/* Inputs */
#define KEY_NONE                0x00
#define KEY_DIT                 0x01
#define KEY_DAH                 0x02
volatile uint8_t Keys;

/* Current WPM setting and the element lengths */
uint8_t Wpm;
uint16_t ElementTimes[3];

/* Input from straight key? */
bool StraightKey;

/* Paddles swapped? */
bool PaddleSwapped;

/* Code elements */
#define         EL_PAUSE        0x00U
#define         EL_DIT          0x01U
#define         EL_DAH          0x02U
#define         EL_INV          0x03U   /* Invalid */

/* One CW encoded character. Uses up to 8 two bit code elements in an uint16_t.
 * Last element in lsb. Unused elements are filled with EL_PAUSE.
 */
typedef uint16_t CwChar;

/* Macros to generate characters consisting of some number of elements */
#define CWC_1(e1) \
        (e1)
#define CWC_2(e1, e2) \
        (((e1) << 2) | (e2))
#define CWC_3(e1, e2, e3) \
        (((e1) << 4) | ((e2) << 2) | (e3))
#define CWC_4(e1, e2, e3, e4) \
        (((e1) << 6) | ((e2) << 4) | ((e3) << 2) | (e4))
#define CWC_5(e1, e2, e3, e4, e5) \
        (((e1) << 8) | ((e2) << 6) | ((e3) << 4) | ((e4) << 2) | (e5))
#define CWC_6(e1, e2, e3, e4, e5, e6) \
        (((e1) << 10) | ((e2) << 8) | ((e3) << 6) | ((e4) << 4) | ((e5) << 2) | (e6))

/* Macros for some CW characters */
#define CW_INV    0xFFFF
#define CW_DASH   CWC_6(EL_DAH, EL_DIT, EL_DIT, EL_DIT, EL_DIT, EL_DAH)
#define CW_STROKE CWC_5(EL_DAH, EL_DIT, EL_DIT, EL_DAH, EL_DIT)
#define CW_1      CWC_5(EL_DIT, EL_DAH, EL_DAH, EL_DAH, EL_DAH)
#define CW_2      CWC_5(EL_DIT, EL_DIT, EL_DAH, EL_DAH, EL_DAH)
#define CW_3      CWC_5(EL_DIT, EL_DIT, EL_DIT, EL_DAH, EL_DAH)
#define CW_4      CWC_5(EL_DIT, EL_DIT, EL_DIT, EL_DIT, EL_DAH)
#define CW_5      CWC_5(EL_DIT, EL_DIT, EL_DIT, EL_DIT, EL_DIT)
#define CW_6      CWC_5(EL_DAH, EL_DIT, EL_DIT, EL_DIT, EL_DIT)
#define CW_7      CWC_5(EL_DAH, EL_DAH, EL_DIT, EL_DIT, EL_DIT)
#define CW_8      CWC_5(EL_DAH, EL_DAH, EL_DAH, EL_DIT, EL_DIT)
#define CW_9      CWC_5(EL_DAH, EL_DAH, EL_DAH, EL_DAH, EL_DIT)
#define CW_0      CWC_5(EL_DAH, EL_DAH, EL_DAH, EL_DAH, EL_DAH)
#define CW_QM     CWC_6(EL_DIT, EL_DIT, EL_DAH, EL_DAH, EL_DIT, EL_DIT)
#define CW_A      CWC_2(EL_DIT, EL_DAH)
#define CW_B      CWC_4(EL_DAH, EL_DIT, EL_DIT, EL_DIT)
#define CW_C      CWC_4(EL_DAH, EL_DIT, EL_DAH, EL_DIT)
#define CW_D      CWC_3(EL_DAH, EL_DIT, EL_DIT)
#define CW_E      CWC_1(EL_DIT)
#define CW_F      CWC_4(EL_DIT, EL_DIT, EL_DAH, EL_DIT)
#define CW_G      CWC_3(EL_DAH, EL_DAH, EL_DIT)
#define CW_H      CWC_4(EL_DIT, EL_DIT, EL_DIT, EL_DIT)
#define CW_I      CWC_2(EL_DIT, EL_DIT)
#define CW_J      CWC_4(EL_DIT, EL_DAH, EL_DAH, EL_DAH)
#define CW_K      CWC_3(EL_DAH, EL_DIT, EL_DAH)
#define CW_L      CWC_4(EL_DIT, EL_DAH, EL_DIT, EL_DIT)
#define CW_M      CWC_2(EL_DAH, EL_DAH)
#define CW_N      CWC_2(EL_DAH, EL_DIT)
#define CW_O      CWC_3(EL_DAH, EL_DAH, EL_DAH)
#define CW_P      CWC_4(EL_DIT, EL_DAH, EL_DAH, EL_DIT)
#define CW_Q      CWC_4(EL_DAH, EL_DAH, EL_DIT, EL_DAH)
#define CW_R      CWC_3(EL_DIT, EL_DAH, EL_DIT)
#define CW_S      CWC_3(EL_DIT, EL_DIT, EL_DIT)
#define CW_T      CWC_1(EL_DAH)
#define CW_U      CWC_3(EL_DIT, EL_DIT, EL_DAH)
#define CW_V      CWC_4(EL_DIT, EL_DIT, EL_DIT, EL_DAH)
#define CW_W      CWC_3(EL_DIT, EL_DAH, EL_DAH)
#define CW_X      CWC_4(EL_DAH, EL_DIT, EL_DIT,EL_DAH)
#define CW_Y      CWC_4(EL_DAH, EL_DIT, EL_DAH, EL_DAH)
#define CW_Z      CWC_4(EL_DAH, EL_DAH, EL_DIT, EL_DIT)



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupCw(void);
/* Module setup */

void SaveCw(void);
/* Save all cw settings in the eeprom */

void SetCwWpm(uint8_t NewWpm);
/* Change the WPM setting */

static inline uint16_t ElementTime(uint8_t Element)
/* Return the time for one code element */
{
    return ElementTimes[Element];
}

static inline void AddWordPause(void)
/* Add enough pause to convert a character pause (3 dits) into a word pause
 * (7 dits).
 */
{
    Sleep(4 * ElementTime(EL_PAUSE));
}

char CwToAscii(CwChar C);
/* Convert a CW character to its ASCII counterpart. Returns 0xFF if unknown. */

CwChar AsciiToCw(char C);
/* Convert an ASCII character to its CW counterpart. Returns 0 if unknown. */

void PlayCwChar(CwChar C);
/* Play a CW character on the sidetone */

void PlayNumber(uint16_t Number, uint8_t Digits);
/* Play a number with the given digits on the sidetone */

int8_t IsCwDigit(CwChar C);
/* If this is a CW digit, return its value, otherwise return -1 */

static inline bool Dit(void)
/* Check if dit was pressed */
{
    return (Keys & KEY_DIT) != KEY_NONE;
}

static inline bool Dah(void)
/* Check if dah was pressed */
{
    return (Keys & KEY_DAH) != KEY_NONE;
}



/* End of cw.h */
#endif



