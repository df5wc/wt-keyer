/*****************************************************************************/
/*                                                                           */
/*                                  timer.h                                  */
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



#ifndef WTKEYER_TIMER_H
#define WTKEYER_TIMER_H



#include <stdint.h>

/* wt-keyer */
#include "timerdefs.h"



/*****************************************************************************/
/*                                   Data                                    */
/*****************************************************************************/



/* Software timer types */
typedef uint16_t Timer;

/* Define for times. Decimal places must be in range 0..999 */
#if ((IRQ_HZ % 1000UL) != 0)
  #define MSEC(msec,usec) \
    ((uint16_t)((uint32_t)(msec)*IRQ_HZ/1000UL + (uint32_t)(usec)*IRQ_HZ/1000000UL))
#else
  #define MSEC(msec,usec) \
    ((uint16_t)((msec)*(IRQ_HZ/1000UL) + (uint32_t)(usec)*IRQ_HZ/1000000UL))
#endif



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



void SetupTimer(void);
/* Setup the irq timer ticker */

Timer GetTicks(void);
/* Return the current timer ticks (timer-irq.S) */

static inline Timer StartTimer(void)
/* Return a timer start value */
{
    return (Timer)GetTicks();
}

static inline void IncTimer(Timer* T, uint16_t Ticks)
/* Add a certain amount to a timer */
{
    *T += Ticks;
}

static inline uint16_t ElapsedTime(Timer T)
/* Return the elapsed time since start of the timer. Result is in IRQ_HZ
 * units.
 */
{
    return GetTicks() - T;
}

void Sleep(uint16_t Ticks);
/* Sleep for a certain amount of time */



/* End of timer.h */
#endif



