/*****************************************************************************/
/*                                                                           */
/*                                  keyer.c                                  */
/*                                                                           */
/*                                 CW keyer                                  */
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
#include <stdint.h>
#include <avr/pgmspace.h>

/* wt-keyer */
#include "cw.h"
#include "eeprom.h"
#include "keyer.h"
#include "timer.h"
#include "txbuffer.h"
#include "tone.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* Enable dash/dot memory */
static uint8_t eeKeyerMemory EEMEM;
bool KeyerMemory;

/* Keyer states */
enum States {
    ST_SETUP,
    ST_IDLE,
    ST_EL_START,
    ST_EL,
    ST_PAUSE,
    ST_SK_OFF,
    ST_SK_ON,
};
typedef enum States States;
static uint8_t State;

/* Keyer variables */
static uint8_t Element;
static uint8_t NextElement;
static Timer T;
static uint16_t WaitTime;

/* Character buffer */
static CwChar CharBuf;
static bool CharComplete;


/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupKeyer(void)
/* Module setup */
{
    /* Read the settings from the eeprom */
    KeyerMemory = EepromReadByte(&eeKeyerMemory, true);

    /* Initialize variables */
    TxBufClear(&TxBuf);
    State = ST_SETUP;
    CharBuf = 0x0000;
    CharComplete = false;
}



void SaveKeyer(void)
/* Save keyer settings to eeprom */
{
    EepromWriteByte(&eeKeyerMemory, KeyerMemory);
}



CwChar GetKeyedChar(void)
/* Return and clear the keyed character */
{
    CwChar C = CharBuf;
    CharBuf = 0x0000;
    CharComplete = false;
    return C;
}



static void AddElement(uint8_t E)
/* Add an element to the character buffer */
{
    /* If the character was complete before and we get a new element this
     * means that the character wasn't retrieved. Just clear it and start
     * over.
     */
    if (CharComplete) {
        CharComplete = false;
        CharBuf = E;
        return;
    }

    /* We cannot add something to an already invalid char */
    if (CharBuf == CW_INV) {
        return;
    }

    /* Check if there is space for another element */
    if (CharBuf & 0xC000) {
        /* No space left */
        CharBuf = CW_INV;
        return;
    }

    /* Add the new element */
    CharBuf = (CharBuf << 2) | E;
}



bool Keyer(void)
/* Run the keyer. Must be called in regular intervals. Returns true if a
 * decode input character is waiting that may be retrieved and cleared by
 * GetKeyedChar().
 */
{
    do {
        switch (State) {

            case ST_SETUP:
                if (StraightKey) {
                    State = ST_SK_OFF;
                    continue;
                } else {
                    /* Initialize for ST_IDLE */
                    T = StartTimer();
                    State = ST_IDLE;
                    /* FALLTHROUGH */
                }

            case ST_IDLE:
                /* If we have elements in the character buffer and we wait for
                 * another dit length, we had a total of 2 dit lengths pause
                 * since the last element and assume that the input character
                 * is complete.
                 */
                if (CharBuf && ElapsedTime(T) >= ElementTime(EL_PAUSE)) {
                    CharComplete = true;
                }
                /* Check for paddle key presses */
                if (Keys & KEY_DIT) {
                    Element = EL_DIT;
                    State = ST_EL_START;
                    continue;
                } else if (Keys & KEY_DAH) {
                    Element = EL_DAH;
                    State = ST_EL_START;
                    continue;
                }
                break;

            case ST_EL_START:
                /* Start the element stored in "Element" */
                SideToneStart();
                TxBufPush(&TxBuf, StartTimer(), true);
                T = StartTimer();
                WaitTime = ElementTime(Element);
                State = ST_EL;
                NextElement = EL_PAUSE;
                break;

            case ST_EL:
                /* We are inside an element. */
                if (KeyerMemory) {
                    if (Dit() && Element == EL_DAH) {
                        NextElement = EL_DIT;
                    } else if (Dah() && Element == EL_DIT) {
                        NextElement = EL_DAH;
                    }
                }
                if (ElapsedTime(T) >= WaitTime) {
                    AddElement(Element);
                    SideToneDone();
                    TxBufPush(&TxBuf, StartTimer(), false);
                    T = StartTimer();
                    WaitTime = ElementTime(EL_PAUSE);
                    State = ST_PAUSE;
                }
                break;

            case ST_PAUSE:
                /* Pause after dit or dah */
                if (Dit() && Element == EL_DAH) {
                    NextElement = EL_DIT;
                } else if (Dah() && Element == EL_DIT) {
                    NextElement = EL_DAH;
                }
                if (ElapsedTime(T) >= WaitTime) {
                    Element = NextElement;
                    if (Element == EL_PAUSE) {
                        State = ST_SETUP;
                    } else {
                        WaitTime = ElementTime(Element);
                        State = ST_EL_START;
                        continue;
                    }
                }
                break;

            case ST_SK_OFF:
                /* Straight key and key open */
                if (Keys & KEY_DIT) {
                    /* Key was closed */
                    SideToneStart();
                    TxBufPush(&TxBuf, StartTimer(), true);
                    State = ST_SK_ON;
                }
                break;

            case ST_SK_ON:
                /* Straight key and key closed */
                if ((Keys & KEY_DIT) == 0) {
                    /* Key was opened */
                    SideToneDone();
                    TxBufPush(&TxBuf, StartTimer(), false);
                    State = ST_SK_OFF;
                }
                break;

        }
    } while (false);

    return CharComplete;
}



