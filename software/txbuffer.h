/*****************************************************************************/
/*                                                                           */
/*                                txbuffer.h                                 */
/*                                                                           */
/*            Delayed transmit buffer for the walkie-talkie keyer            */
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



#ifndef WTKEYER_TXBUFFER_H
#define WTKEYER_TXBUFFER_H



#include <stdbool.h>
#include <stdint.h>

/* wt-keyer.h */
#include "timer.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* Transmit buffer */
typedef struct {
    bool        On;
    Timer       Time;
} TxBufferEntry;
typedef struct {
    uint8_t         Count;      /* Number of elements in the buffer */
    uint8_t         In;         /* Input pointer */
    uint8_t         Out;        /* Output pointer */
    TxBufferEntry   Buf[16];    /* Element buffer. Must be 2^n in size. */
} TxBuffer;
TxBuffer TxBuf;



/*****************************************************************************/
/*                        Transmit buffer management                         */
/*****************************************************************************/



static inline void TxBufClear(TxBuffer* B)
{
    B->Count = 0;
    B->In    = 0;
    B->Out   = 0;
}

static inline void TxBufPush(TxBuffer* B, Timer T, bool On)
{
    /* If the buffer is full, we overwrite existing data. That's not a problem
     * since this happens in situations when the buffer is unused anyway.
     */
    B->Buf[B->In] = (TxBufferEntry){ .On = On, .Time = T };
    B->In = (B->In + 1) & (sizeof(B->Buf)/sizeof(B->Buf[0]) - 1);
    if (B->Count < sizeof(B->Buf)/sizeof(B->Buf[0])) {
        ++B->Count;
    }
}

static inline const TxBufferEntry* TxBufOut(TxBuffer* B)
{
    return (B->Count > 0)? &B->Buf[B->Out] : 0;
}

static inline void TxBufDrop(TxBuffer* B)
{
    B->Out = (B->Out + 1) & (sizeof(B->Buf)/sizeof(B->Buf[0]) - 1);
    --B->Count;
}

static inline uint8_t TxBufCount(const TxBuffer* B)
{
    return B->Count;
}



/* End of txbuffer.h */
#endif



