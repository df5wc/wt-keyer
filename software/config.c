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



static uint8_t HandleDQuery(uint16_t Unused __attribute__((unused)))
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



static uint8_t HandleIA(uint16_t Unused __attribute__((unused)))
/* Handle the IA (enable iabmic a) command */
{
    KeyerMode = KM_IAMBIC_A;
    SaveKeyer();
    return CMD_OK;
}



static uint8_t HandleIB(uint16_t Unused __attribute__((unused)))
/* Handle the IB (enable iabmic b) command */
{
    KeyerMode = KM_IAMBIC_B;
    SaveKeyer();
    return CMD_OK;
}



static uint8_t HandleIP(uint16_t Unused __attribute__((unused)))
/* Handle the IP (enable plain iabmic) command */
{
    KeyerMode = KM_IAMBIC;
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
    } while (CwMemIsPlaying() && Buttons == BUTTON_C && Keys == KEY_NONE);
    AbortPlayCwMem();
    AddWordPause();
    return CMD_OK;
}



static uint8_t HandleMQuery1(uint16_t Unused __attribute__((unused)))
/* Handle the M?1 (query cw memory 1) command */
{
    return HandleMQuery(CWMEM_1);
}



static uint8_t HandleMQuery2(uint16_t Unused __attribute__((unused)))
/* Handle the M?2 (query cw memory 2) command */
{
    return HandleMQuery(CWMEM_2);
}



static uint8_t HandleM(uint8_t Nr)
/* Handle one of the "program memory" commands */
{
    CwMemory M = { .Count = 0, .Buf = { 0 } };
    TxBufferEntry Element = { .On = false };
    bool FirstWait = true;      /* Means: Wating for the first element */
    Timer EndTimer = StartTimer();

    ResetKeyer();
    while (true) {
        /* Releasing the Cmd button will abort memory programming */
        if (Buttons != BUTTON_C) {
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
                uint16_t DitTime = ElementTime(EL_DIT);
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



static uint8_t HandleM1(uint16_t Unused __attribute__((unused)))
/* Handle the M1 (program cw memory 1) command */
{
    return HandleM(CWMEM_1);
}



static uint8_t HandleM2(uint16_t Unused __attribute__((unused)))
/* Handle the M2 (program cw memory 2) command */
{
    return HandleM(CWMEM_2);
}



static uint8_t HandleOQuery(uint16_t Unused __attribute__((unused)))
/* Handle the O? (tx off delay query) command */
{
    AddWordPause();
    PlayNumber(TxOffDelay, 2);
    return CMD_OK;
}



static uint8_t HandleOnn(uint16_t Delay)
/* Handle the Onn (set tx off delay) command */
{
    if ((uint8_t)Delay < TXOFFDELAY_MIN || (uint8_t)Delay > TXOFFDELAY_MAX) {
        return CMD_UNKNOWN;
    } else {
        TxOffDelay = (uint8_t)Delay;
        SaveRigCtrl();
        return CMD_OK;
    }
}



static uint8_t HandleSK(uint16_t Unused __attribute__((unused)))
/* Handle the SK (straight key) command */
{
    StraightKey = true;
    PaddleSwapped = false;      /* Set but don't save */
    return CMD_OK;
}



static uint8_t HandleSWD(uint16_t Unused __attribute__((unused)))
/* Handle the SWD (disable paddle swap) command */
{
    PaddleSwapped = false;
    SaveCw();
    return CMD_OK;
}



static uint8_t HandleSWE(uint16_t Unused __attribute__((unused)))
/* Handle the SWE (enable paddle swap) command */
{
    PaddleSwapped = true;
    SaveCw();
    return CMD_OK;
}



static uint8_t HandleTQuery(uint16_t Unused __attribute__((unused)))
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



static uint8_t HandleTMD(uint16_t Unused __attribute__((unused)))
/* Handle the TMD (training mode disable) command */
{
    TrainingMode = false;
    SaveRigCtrl();
    return CMD_OK;
}



static uint8_t HandleTME(uint16_t Unused __attribute__((unused)))
/* Handle the TME (training mode enable) command */
{
    TrainingMode = true;
    SaveRigCtrl();
    return CMD_OK;
}



static uint8_t HandleVQuery(uint16_t Unused __attribute__((unused)))
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
        if ((Buttons & BUTTON_C) == 0) {
            return CMD_UNKNOWN;
        }
    }
    AddWordPause();
    return CMD_OK;
}



static uint8_t HandleWQuery(uint16_t Unused __attribute__((unused)))
/* Handle the W? (wpm query) command */
{
    AddWordPause();
    PlayNumber(Wpm, 2);
    return CMD_OK;
}



static uint8_t HandleWnn(uint16_t Wpm)
/* Handle the Wnn (set wpm) command */
{
    if ((uint8_t)Wpm < WPM_MIN || (uint8_t)Wpm > WPM_MAX) {
        return CMD_UNKNOWN;
    } else {
        SetCwWpm((uint8_t)Wpm);
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
     * - IA             activate iambic A
     * - IB             activate iambic B
     * - IP             activate plain iambic mode
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
     *
     * We use a completely table based approach to handle commands. Moving the
     * commands that take a numeric argument out of the table saves some space,
     * but the generic approach is shorter and nicer, even if the generated
     * code is slightly larger than handling commands with arguments separately.
     */
    typedef struct {
        uint8_t Len;
        CwChar  Cmd[4];
        uint8_t (*Handler)(uint16_t);
    } CmdEntry;
    static const CmdEntry Cmds[] PROGMEM = {
        { 2, {  CW_D,   CW_QM,                  }, HandleDQuery     },
        { 4, {  CW_D,   CW_DIG, CW_DIG, CW_DIG, }, HandleDnnn       },
        { 2, {  CW_I,   CW_A,                   }, HandleIA         },
        { 2, {  CW_I,   CW_B,                   }, HandleIB         },
        { 2, {  CW_I,   CW_P,                   }, HandleIP         },
        { 3, {  CW_M,   CW_QM,  CW_1,           }, HandleMQuery1    },
        { 3, {  CW_M,   CW_QM,  CW_2,           }, HandleMQuery2    },
        { 2, {  CW_M,   CW_1,                   }, HandleM1         },
        { 2, {  CW_M,   CW_2,                   }, HandleM2         },
        { 2, {  CW_O,   CW_QM,                  }, HandleOQuery     },
        { 3, {  CW_O,   CW_DIG, CW_DIG,         }, HandleOnn        },
        { 3, {  CW_S,   CW_W,   CW_D,           }, HandleSWD        },
        { 3, {  CW_S,   CW_W,   CW_E,           }, HandleSWE        },
        { 2, {  CW_S,   CW_K,                   }, HandleSK         },
        { 2, {  CW_T,   CW_QM,                  }, HandleTQuery     },
        { 4, {  CW_T,   CW_DIG, CW_DIG, CW_DIG, }, HandleTnnn       },
        { 3, {  CW_T,   CW_M,   CW_D,           }, HandleTMD        },
        { 3, {  CW_T,   CW_M,   CW_E,           }, HandleTME        },
        { 2, {  CW_V,   CW_QM,                  }, HandleVQuery     },
        { 2, {  CW_W,   CW_QM,                  }, HandleWQuery     },
        { 3, {  CW_W,   CW_DIG, CW_DIG,         }, HandleWnn        },
    };

    /* Safety */
    if (Len == 0) {
        return CMD_MAYBE;
    }

    /* Search the table */
    uint16_t Num = 0;
    uint8_t Ret = CMD_UNKNOWN;
    for (uint8_t I = 0; I < sizeof(Cmds)/sizeof(Cmds[0]); ++I) {
        const CmdEntry* E = &Cmds[I];
        uint8_t CmdLen = pgm_read_byte(&E->Len);
        if (Len <= CmdLen) {
            for (uint8_t L = 0; L < Len; ++L) {
                CwChar C = pgm_read_word(&E->Cmd[L]);
                CwChar B = Buf[L];
                if (C == CW_DIG) {
                    int8_t Val = IsCwDigit(B);
                    if (Val < 0) {
                        /* Digit required, but we don't have one */
                        goto Next;
                    }
                    Num = Num * 10 + (uint8_t)Val;
                } else if (B != C) {
                    goto Next;
                }
            }
            if (Len == CmdLen) {
                /* We have a full match - call the handler */
                return ((uint8_t (*)(uint16_t))pgm_read_word(&E->Handler))(Num);
            } else {
                /* We have a partial match but keep searching since there may
                 * be a better one.
                 */
                Ret = CMD_MAYBE;
            }
        }
Next:   ;
    }

    return Ret;
}



void Configuration(void)
/* Handle keyer configuration via morse commands */
{
    /* There are no commands with more than 5 cw chars, so this is safe */
    CwChar Buf[5];
    uint8_t CharCount = 0;

    /* Read characters */
    ResetKeyer();
    while (Buttons == BUTTON_C && !StraightKey) {
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



