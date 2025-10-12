# Makefile for the walkie-talkie keyer
#
# Fuses high:
#   RSTDISBL      = 1
#   WDTON         = 1
#   SPIEN         = 0
#   CKOPT         = 0
#   EESAVE        = 1
#   BOOTSZ        = 00
#   BOOTRST       = 1
# Fuses low:
#   BODLEVEL      = 1
#   BODEN         = 0
#   SUT           = 10
#   CKSEL         = 1110
#
# avrdude -c stk500v2 -p m16 -P /dev/ttyS0 -U lfuse:w:0xAE:m -U hfuse:w:0xC9:m
#

AS      = avr-as
ASFLAGS = -Wa,--warn -mmcu=atmega8
CC	= avr-gcc
CFLAGS  = -O2 -mmcu=atmega8 -Wall -Wextra -Wstrict-prototypes -funsigned-char \
          -funsigned-bitfields -fpack-struct -fshort-enums -std=c99 \
          -Wa,-adhlns=$(<:%.c=%.lst) -Wa,--warn

OBJCOPY = avr-objcopy

PROG	= stk500v2
AVR	= m8
GENVER  = bin/genversion
SERIAL  = /dev/ttyUSB0
TARGET	= wt-keyer

#-----------------------------------------------------------------------------
#

AOBJS  	=       timer-irq.o

COBJS  	=       buttons.o       \
                config.o        \
                cw.o            \
                cwmem.o         \
                eeprom.o        \
                keyer.o         \
                main.o          \
                rigctrl.o       \
                timer.o         \
                tone.o          \
                txbuffer.o      \
                version.o

OBJS	= $(AOBJS) $(COBJS)

#-----------------------------------------------------------------------------
#

# Main target - must be first
.PHONY: all
ifeq (.depend,$(wildcard .depend))
all:	version $(TARGET).hex
include .depend
else
all:	depend
	@$(MAKE)
endif

# Version file
.PHONY: version
version:
	$(GENVER) > version.c

version.c:      version

# Make the dependencies
.PHONY: depend dep
depend dep:	$(COBJS:.o=.c)
	@echo "Creating dependency information"
	$(CC) $(CFLAGS) -MM $^ > .depend

#-----------------------------------------------------------------------------
#

%.s:	%.c
	@echo $<
	@$(CC) $(CFLAGS) -S $<

%.o:	%.c
	@echo $<
	@$(CC) $(CFLAGS) -c $<

$(TARGET).hex: 	$(TARGET).out
	@$(OBJCOPY) -R .eeprom -O ihex $(TARGET).out $(TARGET).hex

$(TARGET).out: 	$(OBJS)
	@$(CC) $(CFLAGS) -o $@ -Wl,-Map=$(TARGET).map,--cref $^

$(TARGET).dis:	$(TARGET).out
	avr-objdump -h -S -z $(TARGET).out > $@


#-----------------------------------------------------------------------------
# Pseudo Targets

xfer:	     	$(TARGET).hex
	@avrdude -c $(PROG) -p $(AVR) -P $(SERIAL) -U $^

writefuses:
	@avrdude -c $(PROG) -p $(AVR) -P $(SERIAL) -U lfuse:w:0xAE:m -U hfuse:w:0xC9:m

readfuses:
	@avrdude -c $(PROG) -p $(AVR) -P $(SERIAL) -U lfuse:r:-:h -U hfuse:r:-:h

eepromread:
	@avrdude -c $(PROG) -p $(AVR) -P $(SERIAL) -U eeprom:r:eeprom.dat:h

dis:	$(TARGET).dis


.PHONY:	size
size:	$(TARGET).out
	@avr-size $(TARGET).out

.PHONY: clean
clean:
	@rm -f *~ core *.map

.PHONY: zap
zap:	clean
	@rm -f .depend $(OBJS) $(COBJS:.o=.lst) $(TARGET).dis $(TARGET).hex $(TARGET).map $(TARGET).out



