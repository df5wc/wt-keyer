/*****************************************************************************/
/*                                                                           */
/*                                 config.c                                  */
/*                                                                           */
/*         Handle configuration commands for the walkie-talkie keyer         */
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



#include <avr/sleep.h>

/* wt-keyer */
#include "buttons.h"
#include "config.h"
#include "cwmem.h"
#include "keyer.h"
#include "rigctrl.h"
#include "tone.h"
#include "txbuffer.h"
#include "version.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* Result codes for the HandleCmd() function */
#define CMD_OK          0       /* Is a command and was handled */
#define CMD_MAYBE       1       /* Maybe a command but we don't know */
#define CMD_UNKNOWN     2       /* Is not a command */



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



static uint8_t HandleDQuery(void)
/* Handle the D? (tx delay query) command */
{
    /* Convert the TX delay into milliseconds */
    uint16_t Val = (uint16_t)((TxDelay * 1000UL) / IRQ_HZ);

    /* Output it */
    AddWordPause();
    PlayNumber(Val, 3);
    return CMD_OK;
}



static uint8_t HandleDnnn(uint16_t Delay)
/* Handle the Dnnn (set tx delay) command */
{
    if (Delay < TXDELAY_MIN || Delay > TXDELAY_MAX) {
        return CMD_UNKNOWN;
    } else {
        TxDelay = MSEC(Delay, 0);       /* Convert to ticks */
        SaveRigCtrl();
        return CMD_OK;
    }
}



static uint8_t HandleIA(void)
/* Handle the IA (enable iabmic a) command */
{
    return CMD_OK;
}



static uint8_t HandleIB(void)
/* Handle the IB (enable iabmic b) command */
{
    return CMD_OK;
}



static uint8_t HandleKMD(void)
/* Handle the KMD (keyer memory disable) command */
{
    KeyerMemory = false;
    SaveKeyer();
    return CMD_OK;
}



static uint8_t HandleKME(void)
/* Handle the KME (keyer memory enable) command */
{
    KeyerMemory = true;
    SaveKeyer();
    return CMD_OK;
}



static uint8_t HandleMQuery(uint8_t Nr)
/* Handle one of the "query memory" commands */
{
    StartPlayCwMem(Nr);
    do {
        sleep_cpu();
        PlayCwMem();
    } while (CwMemIsPlaying() && Buttons == BUTTON_CMD && Keys == KEY_NONE);
    AbortPlayCwMem();
    AddWordPause();
    return CMD_OK;
}



static uint8_t HandleMQuery1(void)
/* Handle the M?1 (query cw memory 1) command */
{
    return HandleMQuery(CWMEM_1);
}



static uint8_t HandleMQuery2(void)
/* Handle the M?2 (query cw memory 2) command */
{
    return HandleMQuery(CWMEM_2);
}



static uint8_t HandleM(uint8_t Nr)
/* Handle one of the "program memory" commands */
{
    CwMemory M = { .Count = 0, .Buf = { 0 } };
    bool FirstWait = true;      /* Means: Wating for the first element */
    Timer EndTimer = StartTimer();
    TxBufferEntry Element = { .On = false };
    uint16_t DitTime = ElementTime(EL_DIT);

    ResetKeyer();
    while (true) {
        /* Releasing the Cmd button will abort memory programming */
        if (Buttons != BUTTON_CMD) {
            return CMD_UNKNOWN;
        }

        /* If we didn't get any input for 2 seconds, we're done */
        if (ElapsedTime(EndTimer) >= MSEC(2000, 0)) {
            /* Reset the keyer to remove any waiting stuff */
            ResetKeyer();

            /* There cannot be any non pause elements waiting, since a dash
             * is 3 dit times max so the end timer cannot be triggered in
             * this time. But we may have no input characters in which case
             * we don't change the memory.
             */
            if (M.Count > 0) {
                SaveCwMem(Nr, &M);
                return CMD_OK;
            } else {
                return CMD_UNKNOWN;
            }
        }

        sleep_cpu();
        Keyer();

        /* Wait until we have a state change */
        if (TxBufCount(&TxBuf) == 0) {
            continue;
        }

        /* Action depends on this state change being the first one or not */
        const TxBufferEntry* E = TxBufOut(&TxBuf);
        if (FirstWait) {
            /* If - for whatever reason - we get a pause here, we just
             * continue waiting.
             */
            if (E->On) {
                FirstWait = false;
                Element = *E;
            }
        } else {
            /* This shouldn't happen, but for safety ignore state changes that
             * aren't really such.
             */
            if (E->On != Element.On) {
                /* Reset the end timer */
                EndTimer = StartTimer();

                /* Calculate how many dit times the old state was active, then
                 * add memory elements according to that time. Using 8 bit
                 * calculations is sufficient here.
                 */
                uint8_t Dits = (ElapsedTime(Element.Time) + DitTime/2) / DitTime;
                while (Dits) {
                    /* We can add 8 dit times per entry at maximum */
                    uint8_t Data = Element.On? 0x01 : 0x00;
                    uint8_t DitsToAdd = (Dits <= 8)? Dits : 8;
                    Data |= (DitsToAdd - 1) << 1;
                    if (!AddToCwMemory(&M, Data)) {
                        /* Memory overflow */
                        return CMD_UNKNOWN;
                    }
                    Dits -= DitsToAdd;
                }
                Element = *E;
            }
        }

        /* Drop the just handled state change */
        TxBufDrop(&TxBuf);
    }
}



static uint8_t HandleM1(void)
/* Handle the M1 (program cw memory 1) command */
{
    return HandleM(CWMEM_1);
}



static uint8_t HandleM2(void)
/* Handle the M2 (program cw memory 2) command */
{
    return HandleM(CWMEM_2);
}



static uint8_t HandleOQuery(void)
/* Handle the O? (tx off delay query) command */
{
    AddWordPause();
    PlayNumber(TxOffDelay, 2);
    return CMD_OK;
}



static uint8_t HandleOnn(uint8_t Delay)
/* Handle the Onn (set tx off delay) command */
{
    if (Delay < TXOFFDELAY_MIN || Delay > TXOFFDELAY_MAX) {
        return CMD_UNKNOWN;
    } else {
        TxOffDelay = Delay;
        SaveRigCtrl();
        return CMD_OK;
    }
}



static uint8_t HandleSK(void)
/* Handle the SK (straight key) command */
{
    StraightKey = true;
    PaddleSwapped = false;      /* Set but don't save */
    return CMD_OK;
}



static uint8_t HandleSWD(void)
/* Handle the SWD (disable paddle swap) command */
{
    PaddleSwapped = false;
    SaveCw();
    return CMD_OK;
}



static uint8_t HandleSWE(void)
/* Handle the SWE (enable paddle swap) command */
{
    PaddleSwapped = true;
    SaveCw();
    return CMD_OK;
}



static uint8_t HandleTQuery(void)
/* Handle the T? (tone frequency query) command */
{
    AddWordPause();
    PlayNumber(ToneFreq, 3);
    return CMD_OK;
}



static uint8_t HandleTnnn(uint16_t Freq)
/* Handle the Tnnn (set tone frequency) command */
{
    if (Freq < TONEFREQ_MIN || Freq > TONEFREQ_MAX) {
        return CMD_UNKNOWN;
    } else {
        SetToneFreq(Freq);
        SaveTone();
        return CMD_OK;
    }
}



static uint8_t HandleTMD(void)
/* Handle the TMD (training mode disable) command */
{
    TrainingMode = false;
    SaveRigCtrl();
    return CMD_OK;
}



static uint8_t HandleTME(void)
/* Handle the TME (training mode enable) command */
{
    TrainingMode = true;
    SaveRigCtrl();
    return CMD_OK;
}



static uint8_t HandleVQuery(void)
/* Handle the V? (version query) command */
{
    const char* V = SVNRev;
    char C;
    AddWordPause();
    while ((C = pgm_read_byte(V++)) != '\0') {
        if (C == ' ') {
            AddWordPause();
        } else {
            PlayCwChar(AsciiToCw(C));
        }
        /* Since the output is quite long, allow to abort it */
        if ((Buttons & BUTTON_CMD) == 0) {
            return CMD_UNKNOWN;
        }
    }
    AddWordPause();
    return CMD_OK;
}



static uint8_t HandleWQuery(void)
/* Handle the W? (wpm query) command */
{
    AddWordPause();
    PlayNumber(Wpm, 2);
    return CMD_OK;
}



static uint8_t HandleWnn(uint8_t Wpm)
/* Handle the Wnn (set wpm) command */
{
    if (Wpm < WPM_MIN || Wpm > WPM_MAX) {
        return CMD_UNKNOWN;
    } else {
        SetCwWpm(Wpm);
        SaveCw();
        return CMD_OK;
    }
}



static uint8_t HandleCmd(CwChar* Buf, uint8_t Len)
/* Handle a command and return one of the CMD_xxx codes */
{
    /* Command list:
     * - D?             query the tx delay
     * - Dnnn           set tx delay in ms
     * - KMD            disable keyer memory
     * - KME            enable keyer memory
     * - IA             activate iambic A
     * - IB             activate iambic B
     * - M?1            query cw memory 1
     * - M?2            query cw memory 2
     * - M1...          program cw memory 1
     * - M2...          program cw memory 2
     * - O?             query tx off delay in dits
     * - Onn            set tx off delay in dits
     * - SK             switch to straight key
     * - SWD            disable paddle swap
     * - SWE            enable paddle swap
     * - T?             query the tone frequency
     * - TMD            disable training mode (TX)
     * - TME            enable training mode (no TX)
     * - Tnnn           set a tone frequency
     * - V?             query the software version number
     * - W?             query the keyer speed
     * - Wnn            set the keyer speed in wpm
     */
    typedef struct {
        uint8_t Len;
        CwChar  Cmd[3];
        uint8_t (*Handler)(void);
    } CmdEntry;
    static const CmdEntry Cmds[] PROGMEM = {
        { 2, { CW_D,  CW_QM         }, HandleDQuery     },
        { 2, { CW_I,  CW_A          }, HandleIA         },
        { 2, { CW_I,  CW_B          }, HandleIB         },
        { 3, { CW_K,  CW_M,  CW_D   }, HandleKMD        },
        { 3, { CW_K,  CW_M,  CW_E   }, HandleKME        },
        { 3, { CW_M,  CW_QM, CW_1   }, HandleMQuery1    },
        { 3, { CW_M,  CW_QM, CW_2   }, HandleMQuery2    },
        { 2, { CW_M,  CW_1          }, HandleM1         },
        { 2, { CW_M,  CW_2          }, HandleM2         },
        { 2, { CW_O,  CW_QM         }, HandleOQuery     },
        { 3, { CW_S,  CW_W,  CW_D   }, HandleSWD        },
        { 3, { CW_S,  CW_W,  CW_E   }, HandleSWE        },
        { 2, { CW_S,  CW_K          }, HandleSK         },
        { 2, { CW_T,  CW_QM         }, HandleTQuery     },
        { 3, { CW_T,  CW_M,  CW_D   }, HandleTMD        },
        { 3, { CW_T,  CW_M,  CW_E   }, HandleTME        },
        { 2, { CW_V,  CW_QM         }, HandleVQuery     },
        { 2, { CW_W,  CW_QM         }, HandleWQuery     },
    };

    /* Safety */
    if (Len == 0) {
        return CMD_MAYBE;
    }

    /* First search in the table */
    uint8_t Ret = CMD_UNKNOWN;
    for (uint8_t I = 0; I < sizeof(Cmds)/sizeof(Cmds[0]); ++I) {
        const CmdEntry* E = &Cmds[I];
        uint8_t CmdLen = pgm_read_byte(&E->Len);
        if (Len <= CmdLen) {
            for (uint8_t L = 0; L < Len; ++L) {
                if (Buf[L] != pgm_read_word(&E->Cmd[L])) {
                    goto Next;
                }
            }
            if (Len == CmdLen) {
                /* We have a full match - call the handler */
                return ((uint8_t (*)(void))pgm_read_word(&E->Handler))();
            } else {
                /* We have a partial match */
                Ret = CMD_MAYBE;
            }
        }
Next:   ;
    }

    /* If we had a partial match, return that */
    if (Ret != CMD_UNKNOWN) {
        return Ret;
    }

    /* Otherwise the command is not in the table, so we must have one of Dnnn,
     * Onn, Tnnn or Wnn.
     */
    if (Buf[0] != CW_D && Buf[0] != CW_O && Buf[0] != CW_T && Buf[0] != CW_W) {
        return CMD_UNKNOWN;
    }
    if (Len == 1) {
        return CMD_MAYBE;
    }
    int8_t DigA = IsCwDigit(Buf[1]);
    if (DigA < 0) {
        return CMD_UNKNOWN;
    }
    if (Len == 2) {
        return CMD_MAYBE;
    }
    int8_t DigB = IsCwDigit(Buf[2]);
    if (DigB < 0) {
        return CMD_UNKNOWN;
    }
    if (Len == 3) {
        uint8_t Val = DigA * 10 + DigB;
        if (Buf[0] == CW_O) {
            return HandleOnn(Val);
        } else if (Buf[0] == CW_W) {
            return HandleWnn(Val);
        } else {
            return CMD_MAYBE;
        }
    }
    int8_t DigC = IsCwDigit(Buf[3]);
    if (DigC < 0) {
        return CMD_UNKNOWN;
    }
    uint16_t Arg = DigA * 100 + DigB * 10 + DigC;
    if (Buf[0] == CW_D) {
        return HandleDnnn(Arg);
    } else {
        return HandleTnnn(Arg);
    }
}



void Configuration(void)
/* Handle keyer configuration via morse commands */
{
    /* There are no commands with more than 5 cw chars, so this is safe */
    CwChar Buf[5];
    uint8_t CharCount = 0;

    /* Read characters */
    ResetKeyer();
    while (Buttons == BUTTON_CMD && !StraightKey) {
        if (Keyer()) {
            /* A decoded input character is waiting. Remember it. */
            Buf[CharCount++] = GetKeyedChar();
            /* Check if we know the command */
            switch (HandleCmd(Buf, CharCount)) {
                case CMD_OK:
                    /* The command is known and was handled */
                    SuccessTone();
                    CharCount = 0;
                    break;
                case CMD_MAYBE:
                    /* This may be a command but we don't know yet */
                    break;
                default:
                    /* The command is definitely unknown */
                    FailureTone();
                    CharCount = 0;
                    break;
            }
        }
        sleep_cpu();
    }

    /* If a partial command was aborted, signal that */
    if (CharCount > 0) {
        FailureTone();
    }
}



