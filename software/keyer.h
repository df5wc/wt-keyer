/*****************************************************************************/
/*                                                                           */
/*                                  keyer.h                                  */
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



#ifndef WTKEYER_KEYER_H
#define WTKEYER_KEYER_H



#include <stdbool.h>

/* wt-keyer.h */
#include "cw.h"
#include "timer.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* Enable dash/dot memory */
bool KeyerMemory;



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupKeyer(void);
/* Module setup */

static inline void ResetKeyer(void)
/* Reset keyer memory and initialize. Same as Setup. */
{
    SetupKeyer();
}

void SaveKeyer(void);
/* Save keyer settings to eeprom */

CwChar GetKeyedChar(void);
/* Return and clear the keyed character */

bool Keyer(void);
/* Run the keyer. Must be called in regular intervals. Returns true if a
 * decode input character is waiting that may be retrieved and cleared by
 * GetKeyedChar().
 */



/* End of keyer.h */
#endif




