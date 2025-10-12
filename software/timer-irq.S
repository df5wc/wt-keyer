#include <avr/io.h>


;----------------------------------------------------------------------------
; Variables

.data

; Timer
.comm           Ticks, 2                ; Timer ticks
Slow:           .byte   0               ; Helper for slow ticker
InWpm:          .byte   0
InPitch:        .byte   0
InCmd:          .byte   0

; External variables defined in the C code
.extern         Keys
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

; Read the input port. Paddle inputs are handled first without debouncing.

        in      r22, _SFR_IO_ADDR(PIND)
        com     r22                     ; Invert so 1 is active
        lds     r24, PaddleSwapped
        cpi     r24, 0
        breq    L2

        bst     r22, 1
        lsl     r22
        bld     r22, 0

L2:     andi    r22, 0x03               ; Mask relevant bits
        sts     Keys, r22               ; Store them for the main program

; Handle port input and debouncing each millisecond. We do this only for the
; switches, not for the paddle. Active keys read as zero on input. We shift
; the bit for each input into its own variable and check if the active state
; was read 8 times in a row. If so, we mark the switch as "pressed".

        lds     r23, Slow
        inc     r23
        cpi     r23, 4                  ; 4*250us = 1ms
        brne    nslow

        in      r22, _SFR_IO_ADDR(PIND)
        clr     r24                     ; Buttons are remembered here

        lds     r23, InCmd
        rol     r22                     ; Bit 7
        ror     r23
        sts     InCmd, r23
        brne    L3
        ori     r24, 0x80

L3:     lds     r23, InPitch
        rol     r22                     ; Bit 6
        ror     r23
        sts     InPitch, r23
        brne    L4
        ori     r24, 0x40

L4:     lds     r23, InWpm
        rol     r22                     ; Bit 5
        ror     r23
        sts     InWpm, r23
        brne    L5
        ori     r24, 0x20

L5:     mov     r22, r24
        lds     r23, Buttons
        eor     r22, r23
        sts     ChangedButtons, r22
        sts     Buttons, r24

; Reset the value for the slow ticker

        clr     r23
nslow:  sts     Slow, r23

; Restore registers and terminate the IRQ handler

irq_end:
        pop     r22
        out     _SFR_IO_ADDR(SREG), r22
        pop     r25
        pop     r24
        pop     r23
        pop     r22
        reti

.end



