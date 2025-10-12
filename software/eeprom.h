/*****************************************************************************/
/*                                                                           */
/*                                  eeprom.h                                 */
/*                                                                           */
/*                               EEPROM access                               */
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



#ifndef WTKEYER_EEPROM_H
#define WTKEYER_EEPROM_H



#include <avr/eeprom.h>
#include <stdint.h>



/*****************************************************************************/
/*                                   Code                                    */
/*****************************************************************************/



uint8_t EepromReadByte(const uint8_t* Addr, uint8_t Default);
/* Read a byte from the EEPROM. Return the default value if the byte at the
 * given address is invalid.
 */

uint16_t EepromReadWord(const uint16_t* Addr, uint16_t Default);
/* Read a word from the EEPROM. Return the default value if the word at the
 * given address is invalid.
 */

static inline void EepromWriteByte(uint8_t* Addr, uint8_t Val)
/* Write a byte to the EEPROM. */
{
    eeprom_update_byte(Addr, Val);
}

static inline void EepromWriteWord(uint16_t* Addr, uint16_t Val)
/* Write a word to the EEPROM */
{
    eeprom_update_word(Addr, Val);
}



/* End of eeprom.h */
#endif




