/*****************************************************************************/
/*                                                                           */
/*                                  cwmem.c                                  */
/*                                                                           */
/*             CW memory management for the walkie-talkie keyer              */
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
#include "cw.h"
#include "cwmem.h"
#include "eeprom.h"
#include "timer.h"
#include "txbuffer.h"
#include "tone.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* The actual memories in eeprom */
static CwMemory eeCwMem[CWMEM_MAX] EEMEM;

/* Cached message lengths */
static uint16_t CwMemLen[CWMEM_MAX];

/* Play state variables */
uint8_t CwMemState;
uint8_t CwMemCur;
uint16_t CwMemIndex;
static uint16_t CwMemWaitTime;
static Timer CwMemTimer;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupCwMem(void)
/* Setup cw memories */
{
    /* We must read both lengths from eeprom to determine if the memories
     * contain data or are uninitialized.
     */
    CwMemLen[0] = EepromReadWord(&eeCwMem[0].Count, 0);
    CwMemLen[1] = EepromReadWord(&eeCwMem[1].Count, 0);
}



void SaveCwMem(uint8_t Number, const CwMemory* M)
/* Save cw memory data to eeprom */
{
    /* Update the data in eeprom */
    uint16_t Count = sizeof(M->Count) + (M->Count + 1) / 2;
    eeprom_update_block(M, &eeCwMem[Number - 1], Count);

    /* Update the cached length */
    CwMemLen[Number - 1] = M->Count;
}



void PlayCwMem(void)
/* Play a cw memory. Places the necessary data into TxBuf so that the morse
 * code can be sent. Must be called in regular intervals until the buffer is
 * output completely.
 */
{
    do {
        const uint8_t* DataPtr;
        uint8_t Data;

        switch (CwMemState) {

            case CMS_IDLE:
                break;

            case CMS_INIT:
                /* Check if we have more data to play */
                if (CwMemIndex >= CwMemLen[CwMemCur - 1]) {
                    /* Done playing this memory */
                    CwMemState = CMS_IDLE;
                    break;
                }

                /* Get a pointer to the next data byte in eeprom */
                DataPtr = eeCwMem[CwMemCur - 1].Buf + CwMemIndex / 2;

                /* Read the next element and bump the data index */
                Data = EepromReadByte(DataPtr, 0x22);
                if (CwMemIndex & 0x01) {
                    /* Relevant data is in the high nibble */
                    Data >>= 4;
                } else {
                    /* Relevant data is in the low nibble */
                    Data &= 0x0F;
                }
                ++CwMemIndex;

                /* Setup the timer, activate the tone */
                CwMemWaitTime = ((Data >> 1) + 1) * ElementTime(EL_DIT);
                CwMemTimer = StartTimer();
                if (Data & 0x01) {
                    TxBufPush(&TxBuf, StartTimer(), true);
                    SideToneStart();
                    CwMemState = CMS_ELEMENT;
                } else {
                    CwMemState = CMS_PAUSE;
                }
                break;

            case CMS_ELEMENT:
                if (ElapsedTime(CwMemTimer) >= CwMemWaitTime) {
                    TxBufPush(&TxBuf, StartTimer(), false);
                    SideToneDone();
                    CwMemState = CMS_INIT;
                    continue;
                }
                break;

            case CMS_PAUSE:
                if (ElapsedTime(CwMemTimer) >= CwMemWaitTime) {
                    CwMemState = CMS_INIT;
                    continue;
                }
                break;

        }

    } while (false);
}



void AbortPlayCwMem(void)
/* Abort playing a cw memory. */
{
    CwMemCur = CWMEM_NONE;
    CwMemIndex = 0;
    CwMemState = CMS_IDLE;
    SideToneDone();             /* In case we were playing */
}



