# Programmer 
PROG = stk500v2
# device the programmer is connected to
PROG_DEV = /dev/tty.usbserial
# Baudrate
AVRDUDE_BAUDRATE = 115200
# Flags
AVRDUDE_FLAGS = -v

# programs
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
AS = avr-as
CP = cp
RM = rm -f
AVRDUDE = avrdude53
SIZE = avr-size

-include $(CURDIR)/config.mk


# flags for the compiler
CFLAGS += -g -O2 -finline-limit=800 -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu99

# flags for the linker
LDFLAGS += -mmcu=$(MCU)

ifneq ($(DEBUG),)
	CFLAGS += -Wall -W -Wchar-subscripts -Wmissing-prototypes
	CFLAGS += -Wmissing-declarations -Wredundant-decls
	CFLAGS += -Wstrict-prototypes -Wshadow -Wbad-function-cast
	CFLAGS += -Winline -Wpointer-arith -Wsign-compare
	CFLAGS += -Wunreachable-code -Wdisabled-optimization
	CFLAGS += -fshort-enums
	CFLAGS += -Wcast-align -Wwrite-strings -Wnested-externs -Wundef
	CFLAGS += -Wa,-adhlns=$(basename $@).lst
	CFLAGS += -DDEBUG
endif


all:

clean:
	$(RM) *.hex *.eep.hex *.o *.lst *.lss


%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@

%.eep.hex: %
	$(OBJCOPY) --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex -j .eeprom $< $@

%.lss: %
	$(OBJDUMP) -h -S $< > $@

%-size: %.hex
	$(SIZE) $<

