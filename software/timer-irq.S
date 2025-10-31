#include <avr/io.h>

#include "buttondefs.h"
#include "timerdefs.h"


;----------------------------------------------------------------------------
; Variables

.data

; Timer
.comm           Ticks, 2                ; Timer ticks
ButtonC:        .byte   0
Button1:        .byte   0
Button2:        .byte   0

; External variables defined in the C code
.extern         Keys
.extern         ChangedKeys
.extern         PaddleSwapped
.extern         Buttons
.extern         ChangedButtons

;----------------------------------------------------------------------------
; Functions for reading variables
;

.text

.global GetTicks
.func   GetTicks
GetTicks:
        cli
        lds     r24, Ticks
        lds     r25, Ticks+1
        sei
        ret
.endfunc

;----------------------------------------------------------------------------
; Interrupt handler
;
;

.text
.global TIMER2_COMP_vect
TIMER2_COMP_vect:

; Be sure to save all registers we're about to use

        push    r22
        push    r23
        push    r24
        push    r25
        in      r22, _SFR_IO_ADDR(SREG)
        push    r22

; Update the timer ticks

        lds     r24, Ticks
        lds     r25, Ticks+1
        adiw    r24, 1
        sts     Ticks, r24
        sts     Ticks+1, r25

; Handle port input. Switches are read each millisecond and debounced for 8
; reads meaning that the main program gets a changed value at most each 8ms.
; Switches are active low. Debouncing is done by shifting the bits into a
; byte variable. If it is zero, the switch has been read active for 8 cycles.
; Paddle inputs are read each millisecond and aren't debounced.

#if IRQ_HZ != 4000
#error "Code assumes IRQ_HZ=4000!"
#endif

        mov     r22, r24
        andi    r22, 0x03               ; Check for 1ms tick
        brne    IrqEnd

; Code below is executed each millisecond

        in      r22, _SFR_IO_ADDR(PIND) ; Read all inputs

; Handle paddle inputs every ms

        com     r22                     ; Invert so 1 is active
        lds     r24, PaddleSwapped
        tst     r24
        breq    NoSwap

        bst     r22, 1                  ; Swap input bits 0 and 1
        lsl     r22
        bld     r22, 0

NoSwap: andi    r22, 0x03               ; Mask relevant bits
        mov     r24, r22
        lds     r23, ChangedKeys
        eor     r22, r23
        sts     Keys, r24               ; Store them for the main program
        sts     ChangedKeys, r22        ; Remember which keys have changed


; Read and debounce the switches

        in      r22, _SFR_IO_ADDR(PIND) ; Reread the inputs
        clr     r24                     ; = BUTTON_NONE

        lds     r23, ButtonC
        rol     r22                     ; Bit 7
        ror     r23
        sts     ButtonC, r23
        brne    L1
        ori     r24, BUTTON_C

L1:     lds     r23, Button1
        rol     r22                     ; Bit 6
        ror     r23
        sts     Button1, r23
        brne    L2
        ori     r24, BUTTON_1

L2:     lds     r23, Button2
        rol     r22                     ; Bit 5
        ror     r23
        sts     Button2, r23
        brne    L3
        ori     r24, BUTTON_2

L3:     mov     r22, r24
        lds     r23, Buttons
        eor     r22, r23
        sts     ChangedButtons, r22
        sts     Buttons, r24

; Restore registers and terminate the IRQ handler

IrqEnd:
        pop     r22
        out     _SFR_IO_ADDR(SREG), r22
        pop     r25
        pop     r24
        pop     r23
        pop     r22
        reti

.end



