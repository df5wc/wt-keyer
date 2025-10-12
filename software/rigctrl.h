/*****************************************************************************/
/*                                                                           */
/*                                 rigctrl.h                                 */
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



#ifndef WTKEYER_RIGCTRL_H
#define WTKEYER_RIGCTRL_H



#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>

/* wt-keyer */
#include "cw.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* Training mode enabled flag */
bool TrainingMode;

/* Delay for sending after tx was enabled. Value for the delay is in ticks,
 * but the limits are in milliseconds since this is what is shown to the
 * user.
 */
uint16_t TxDelay;
#define TXDELAY_MIN             50
#define TXDELAY_MAX             400
#define TXDELAY_DEFAULT         TXDELAY_MAX     /* Safety */

/* Delay for disabling the transmitter if there are no more elements. Value is
 * in dit lengths.
 */
uint8_t TxOffDelay;
#define TXOFFDELAY_MIN          5
#define TXOFFDELAY_MAX          15
#define TXOFFDELAY_DEFAULT      7



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupRigCtrl(void);
/* Setup the rig control functions */

void SaveRigCtrl(void);
/* Save rig control data to eeprom */

void TxSend(void);
/* Send out delayed characters from keyer when not in training mode. Must be
 * called in regular intervals.
 */

void TxAbort(void);
/* Abort sending */



/* End of rigctrl.h */
#endif





