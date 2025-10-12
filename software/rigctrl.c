/*****************************************************************************/
/*                                                                           */
/*                                 rigctrl.c                                 */
/*                                                                           */
/*             Rig control functions for the walkie-talkie keyer             */
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
#include "rigctrl.h"
#include "timer.h"
#include "txbuffer.h"
#include "tone.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* Training mode enabled flag */
static uint8_t eeTrainingMode EEMEM;
bool TrainingMode;

/* Delay for sending after tx was enabled. Value for the delay is in ticks */
static uint16_t eeTxDelay EEMEM;
uint16_t TxDelay;

/* Delay for disabling the transmitter if there are no more elements. Value is
 * in dit lengths.
 */
static uint8_t eeTxOffDelay;
uint8_t TxOffDelay;

/* State variables to control sending */
typedef enum {
    ST_IDLE,
    ST_TXWAIT,
    ST_TONE,
    ST_PAUSE,
} States;
static uint8_t State;
static Timer OffTimer;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupRigCtrl(void)
/* Setup the rig control functions */
{
    TrainingMode = EepromReadByte(&eeTrainingMode, false);
    TxDelay = EepromReadWord(&eeTxDelay, MSEC(TXDELAY_DEFAULT, 0));
    TxOffDelay = EepromReadByte(&eeTxOffDelay, TXOFFDELAY_DEFAULT);
}



void SaveRigCtrl(void)
/* Save rig control data to eeprom */
{
    EepromWriteByte(&eeTrainingMode, TrainingMode);
    EepromWriteWord(&eeTxDelay, TxDelay);
    EepromWriteByte(&eeTxOffDelay, TxOffDelay);
}



static inline void EnableTx(void)
/* Activate PTT */
{
    PORTB |= 0x01;
}



static inline void DisableTx(void)
/* Deactivate PTT. */
{
    PORTB &= ~0x01;
}



void TxSend(void)
/* Send out delayed characters from keyer when not in training mode. Must be
 * called in regular intervals.
 */
{
    /* If we're in training mode, just ignore the call */
    if (TrainingMode) {
        return;
    }

    if (TxBufCount(&TxBuf) > 0) {
        /* Get a pointer to the entry */
        const TxBufferEntry* E = TxBufOut(&TxBuf);
        /* If the keying entry starts a tone, we must enable TX */
        if (E->On && State == ST_IDLE) {
            State = ST_TXWAIT;
            EnableTx();
        } else if (ElapsedTime(E->Time) >= TxDelay) {
            if (E->On) {
                State = ST_TONE;
                TxToneStart();
            } else {
                if (State == ST_TONE) {
                    State = ST_PAUSE;
                    OffTimer = StartTimer();
                }
                TxToneDone();
            }
            /* Drop the entry pointed to by E */
            TxBufDrop(&TxBuf);
        }
    }
    if (State == ST_PAUSE) {
        uint16_t TxOffTime = TxOffDelay * ElementTime(EL_PAUSE);
        if (ElapsedTime(OffTimer) >= TxOffTime) {
            State = ST_IDLE;
            DisableTx();
        }
    }
}



void TxAbort(void)
/* Abort sending */
{
    TxToneDone();
    DisableTx();
    State = ST_IDLE;
}



