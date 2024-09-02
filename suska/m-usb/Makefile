TARGET = main
#BOARD = SUSKA_C
BOARD = AVRCORE
#BOARD = ARDUINO

OPTIONAL = PUZZLE
#OPTIONAL = USB
#OPTIONAL = NOOPTION

SDCARD = YES
#SDCARD = NO

ifeq ($(BOARD),SUSKA_C)
        MCU = atmega644
        F_CPU = 16000000UL
endif

ifeq ($(BOARD),AVRCORE)
        MCU = atmega644p
#	F_CPU = 14745600UL
	F_CPU = 16000000UL
#       F_CPU = 32000000UL
#	F_CPU = 48000000UL
#	F_CPU = 56000000UL
#	F_CPU = 60000000UL
#	F_CPU = 64000000UL
endif

ifeq ($(BOARD),ARDUINO)
        MCU = atmega328p
        F_CPU = 16000000UL
#	F_CPU = 14745600UL
endif


# export current directory to use in sub-makefiles
export CURDIR

# flags for avrdude
ifeq ($(MCU),atmega8)
	AVRDUDE_MCU=m8
endif
ifeq ($(MCU),atmega88)
	AVRDUDE_MCU=m88
endif
ifeq ($(MCU),atmega168)
	AVRDUDE_MCU=m168
endif
ifeq ($(MCU),atmega32)
	AVRDUDE_MCU=m32
endif
ifeq ($(MCU),atmega644)
	AVRDUDE_MCU=m644
endif

# include bootloader stuff
ifeq ($(BOARD),AVRCORE)
include $(CURDIR)/bootloader-altera.mk
else
include $(CURDIR)/bootloader.mk
endif

# include avr-generic makefile configuration
include $(CURDIR)/avr.mk

SRC = $(shell echo *.c)

OBJECTS += ../timer/tick.o 
OBJECTS += ../uart-irq/uart-irq.o 
OBJECTS += ../fifo/fifo.o ../misc/itoa.o ../misc/atoi.o ../misc/ctoHex.o ../misc/f2a.o
ifeq ($(SDCARD),YES)
OBJECTS += ../spi/spi.o 
#OBJECTS += ../spi/softspi.o 
OBJECTS += ../mmc/mmc.o ../mmc/mmc-hw.o ../tff/diskio.o
OBJECTS += ../tff/ff.o ../tff/getfattime.o
#OBJECTS += ../tff/ccsbcs.o
endif
OBJECTS += ../shell/parser.o ../shell/shell.o ../shell/commands.o
OBJECTS += ../suska-iii/suskaspi.o
OBJECTS += ../suska-iii/suskashell.o
OBJECTS += ../coretype/coretypes.o
ifeq ($(OPTIONAL),USB)
OBJECTS += ../usb/usb.o ../usb/hid.o ../usb/hidhandler.o ../usb/usbloop.o
OBJECTS += ../devices/spi/max3421e/max3421e.o ../devices/spi/max3421e/max3421e_usb.o
endif
ifeq ($(OPTIONAL),PUZZLE)
OBJECTS += ../shell/puzzle.o
endif

OBJECTS += shell_poll.o
OBJECTS += ../suska-iii/atarikb.o
OBJECTS += main.o

CFLAGS += -I.. -I$(CURDIR) -I. -D$(BOARD) -D$(OPTIONAL)
ifeq ($(OPTIONAL),USB)
CFLAGS += -DUSB_LOOP
endif

ifeq ($(OPTIONAL),PUZZLE)
CFLAGS += -D__HAVE_FILESYSTEM__
endif

# Additional libraries
#
## Minimalistic printf version
#LDFLAGS += -Wl,-u,vfprintf -lprintf_min
#
## Floating point printf version (requires -lm below)
LDFLAGS +=  -Wl,-u,vfprintf -lprintf_flt
#
## -lm = math library
LIBS = -lm 


# Name of Makefile for make depend
MAKEFILE = Makefile

$(OBJECTS): config.mk


all: $(TARGET).hex $(TARGET).lss $(TARGET).bin
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
#	@echo -n "size is: "
#	@$(SIZE) -A $(TARGET).hex | grep "\.sec1" | tr -s " " | cut -d" " -f2
	@echo "==============================="

$(TARGET): $(OBJECTS) $(TARGET).o
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)


# subdir magic
%/% %/%.o %/%.hex %/all %/depend %/install %/clean:
	$(MAKE) -C $(@D) -e $(@F)


clean: clean-$(TARGET) 

clean-$(TARGET):
	rm -f $(TARGET)
	rm -f $(OBJECTS)

install: $(TARGET).hex
	$(AVRDUDE) -p $(AVRDUDE_MCU) -c $(PROG) -P $(PROG_DEV) -b $(AVRDUDE_BAUDRATE)  $(AVRDUDE_FLAGS) -U f:w:$<
