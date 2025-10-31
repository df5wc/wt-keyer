/*****************************************************************************/
/*                                                                           */
/*                                  timer.c                                  */
/*                                                                           */
/*                Timer functions for the walkie-talkie keyer                */
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



#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

/* wt-keyer */
#include "timer.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* T2 register values */
#define T2_PRESCALER    64UL
#define T2_COMPARE      ((uint8_t) (CLOCK_HZ / (T2_PRESCALER * IRQ_HZ)))



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupTimer(void)
/* Setup the irq timer ticker */
{
    /* Setup timer 2 which generates timing interrupts: CTC mode, interrupt
     * on timer compare, overflow register for irq frequency specified above.
     */
    OCR2 = T2_COMPARE;
    TCCR2  = (0 << FOC2) |
             (1 << WGM21) | (0 << WGM20) |              /* CTC mode */
             (0 << COM21) | (0 << COM20) |              /* OC2 disconnected */
             (1 << CS22) | (0 << CS21) | (0 << CS20);   /* Prescaler 64 */
    TIMSK  = (1 << OCIE2);

    /* Enable sleep mode and interrupts */
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    sei();
}



void Sleep(uint16_t Ticks)
/* Sleep for a certain amount of time */
{
    Timer T = StartTimer();
    do {
        sleep_cpu();
    } while (ElapsedTime(T) < Ticks);
}



