/*****************************************************************************/
/*                                                                           */
/*                                  main.c                                   */
/*                                                                           */
/*                     Dual tone generator main program                      */
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
#include <avr/io.h>
#include <avr/sleep.h>

/* wt-keyer */
#include "buttons.h"
#include "config.h"
#include "cw.h"
#include "cwmem.h"
#include "keyer.h"
#include "rigctrl.h"
#include "timer.h"
#include "tone.h"
#include "version.h"



/*****************************************************************************/
/*                             Helper functions                              */
/*****************************************************************************/



static void SetupIO(void)
/* Setup I/O stuff */
{
    /* IO Ports: PB0 and PB1/PB2 (= OC1A/OC1B) are outputs */
    DDRB = 0x07;

    /* Port C is currently unused and configured as input */
    DDRC = 0x00;
    PORTC = 0x00;       /* Tri-state port C */

    /* Port D is an input. Used bits are PD0/PD1/PD5/PD6/PD7 */
    DDRD = 0x00;
    PORTD = 0xE3;       /* Activate pullups for all used port bits */

    /* Setup tone generation */
    SetupTone();

    /* Setup the timer ticker */
    SetupTimer();
}



/*****************************************************************************/
/*                               Main program                                */
/*****************************************************************************/



int main(void)
{
    /* Initialize I/O and modules */
    SetupIO();
    SetupCw();
    SetupCwMem();
    SetupKeyer();
    SetupRigCtrl();

    /* Run forever */
    while (1) {
        /* Run the keyer. If it is idle, handle switches. React only on button
         * press, not release.
         */
        Keyer();
        uint8_t B;
        if ((B = (ChangedButtons & Buttons)) != BUTTON_NONE) {
            /* Keyer is idle. Force TX and memory play off, then handle
             * switches.
             */
            TxAbort();
            switch (B) {
                case BUTTON_CMD:
                    AbortPlayCwMem();
                    Configuration();
                    break;

                case BUTTON_1:
                    if (CwMemIsPlaying()) {
                        AbortPlayCwMem();
                    } else {
                        StartPlayCwMem(CWMEM_1);
                    }
                    break;

                case BUTTON_2:
                    if (CwMemIsPlaying()) {
                        AbortPlayCwMem();
                    } else {
                        StartPlayCwMem(CWMEM_2);
                    }
                    break;

                default:
                    /* Ignore other button combinations */
                    break;
            }
            /* Reinitialize keyer and buffer */
            ResetKeyer();
        } else {
            /* Play cw memories if active */
            if (CwMemIsPlaying()) {
                /* Allow aborting with the paddle or straight key. Beware: If
                 * a straight key is used, dah may be tied to ground.
                 */
                if (Dit() || (!StraightKey && Dah())) {
                    AbortPlayCwMem();
                } else {
                    PlayCwMem();
                }
            }

            /* Send elements read by the keyer */
            TxSend();
        }

        /* Clear changed buttons and wait for the next timer interrupt */
        ChangedButtons = 0;
        sleep_cpu();
    }
}



