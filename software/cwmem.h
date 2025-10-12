/*****************************************************************************/
/*                                                                           */
/*                                  cwmem.h                                  */
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



#ifndef WTKEYER_CWMEM_H
#define WTKEYER_CWMEM_H



#include <stdbool.h>
#include <stdint.h>



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* Maximum number of elements that can be stored. Each element has one bit
 * that says "active" or not. Three more bits encode the length in dit lengths
 * for the current speed plus one (since we don't need zero length elements).
 * Some examples:
 *
 * dit: length=1, active=1, value = 0x01
 * dah: length=3, active=1, value = 0x06
 * inter element pause: length=1, active=0, value=0x00
 * inter char pause: length=3, active=0, value=0x04
 * inter word pause: length=7, active=0, value=0x0C
 *
 * A pause that is longer than 8 dits would be encoded in more elements.
 * Since we store one such element per nibble, the maximum total number is
 * always even. The ATMega8 has 512 bytes of eeprom which is partly used by
 * other data. So the maximum we can use is somewhere between 400 and 500
 * bytes meaning twice as many elements as defined above.
 */
typedef struct {
    uint16_t    Count;          /* Number of elements(!) stored */
    uint8_t     Buf[198];       /* Make it 200 bytes total */
} CwMemory;
#define CWMEM_MAX_EL    (sizeof(((CwMemory*)0)->Buf) * 2U)

/* Codes for the memories. Zero means "no memory". */
#define CWMEM_NONE      0
#define CWMEM_1         1
#define CWMEM_2         2
#define CWMEM_MAX       2

/* Play state variables */
enum {
    CMS_IDLE,
    CMS_INIT,
    CMS_ELEMENT,
    CMS_PAUSE,
};
uint8_t CwMemState;
uint8_t CwMemCur;
uint16_t CwMemIndex;



/*****************************************************************************/
/*                              struct CwMemory                              */
/*****************************************************************************/



static inline bool AddToCwMemory(CwMemory* M, uint8_t E)
/* Add an entry to a CW memory structure. Returns true if successful, returns
 * false for a memory overflow.
 */
{
    if (M->Count >= CWMEM_MAX_EL) {
        return false;
    }
    uint16_t Index = M->Count / 2;
    if (M->Count & 0x01) {
        /* Data is in high nibble */
        M->Buf[Index] |= (E <<= 4);
    } else {
        /* Data is in low nibble */
        M->Buf[Index] = (E & 0x0F);
    }
    ++M->Count;
    return true;
}



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupCwMem(void);
/* Setup cw memories */

void SaveCwMem(uint8_t Number, const CwMemory* M);
/* Save cw memory data to eeprom */

static inline void StartPlayCwMem(uint8_t Number)
/* Start playing a cw memory buffer */
{
    CwMemCur = Number;
    CwMemIndex = 0;
    CwMemState = CMS_INIT;
}

void PlayCwMem(void);
/* Play a cw memory. Places the necessary data into TxBuf so that the morse
 * code can be sent. Must be called in regular intervals until the buffer is
 * output completely.
 */

void AbortPlayCwMem(void);
/* Abort playing a cw memory. */

static inline bool CwMemIsPlaying(void)
/* Check if we're currently playing back a cw memory. */
{
    return (CwMemState != CMS_IDLE);
}



/* End of cwmem.h */
#endif




