TARGET = main
#BOARD = SUSKA_C
#BOARD = SUSKA_C_SYSCTRL
BOARD = SUSKA_BF
#BOARD = SUSKA_B

OPTIONAL = PUZZLE
#OPTIONAL = NOOPTION

ifeq ($(BOARD),SUSKA_C)
	MCU = atmega644
	F_CPU = 16000000UL
endif

ifeq ($(BOARD),SUSKA_C_SYSCTRL)
	MCU = atmega16
	F_CPU = 8000000UL
endif

ifeq ($(BOARD),SUSKA_B)
	MCU = atmega644
	F_CPU = 8000000UL
endif

ifeq ($(BOARD),SUSKA_BF)
	MCU = atmega649a
	F_CPU = 16000000UL
endif


# microcontroller and project specific settings (everything can be overridden using config.mk)
# normally set by BOARD see above
#F_CPU = 20000000UL
#F_CPU = 1000000UL
#F_CPU = 16000000UL
#MCU = atmega168
#MCU = atmega644
#MCU = atmega8
#MCU = atmega88
#MCU = atmega649A

# export current directory to use in sub-makefiles
export CURDIR

# flags for avrdude
ifeq ($(MCU),atmega16)
	AVRDUDE_MCU=m16
endif
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

ifeq ($(BOARD),SUSKA_BF)
include $(CURDIR)/bootloader-bf.mk
else ifeq ($(BOARD),SUSKA_C_SYSCTRL)
include $(CURDIR)/bootloader-c-sysctrl.mk
else
include $(CURDIR)/bootloader.mk
endif
# include avr-generic makefile configuration
include $(CURDIR)/avr.mk

SRC = $(shell echo *.c)

OBJECTS += ../timer/tick.o 
OBJECTS += ../uart-irq/uart-irq.o 
OBJECTS += ../fifo/fifo.o ../misc/itoa.o ../misc/atoi.o ../misc/ctoHex.o ../misc/f2a.o
OBJECTS += ../adc/adc.o
OBJECTS += ../suska-iii/power.o

OBJECTS += main.o
ifneq ($(BOARD),SUSKA_C_SYSCTRL)
OBJECTS += ../spi/softspi.o 
OBJECTS += ../mmc/mmc-hw.o ../mmc/mmc.o ../tff/diskio.o
OBJECTS += ../tff/ff.o ../tff/getfattime.o
OBJECTS += ../tff/ccsbcs.o
OBJECTS += ../asisp/asisp.o ../asisp/asappl.o ../asisp/asshell.o
OBJECTS += ../suska-iii/suskaspi.o
endif
OBJECTS += ../shell/parser.o ../shell/shell.o ../shell/commands.o
ifeq ($(OPTIONAL),PUZZLE)
OBJECTS += ../shell/puzzle.o
endif
OBJECTS += ../suska-iii/suskashell.o
OBJECTS += ../coretype/coretypes.o

ifeq ($(BOARD),SUSKA_C)
#	OBJECTS += ../usb/usb.o ../usb/hid.o
#	OBJECTS += ../usb/hidhandler.o ../usb/usbloop.o
#	OBJECTS += ../devices/spi/max3421e/max3421e.o
#	OBJECTS += ../devices/spi/max3421e/max3421e_usb.o
	OBJECTS += shell_poll.o ../suska-iii/sdrawfile.o
endif

ifeq ($(BOARD),SUSKA_C_SYSCTRL)
	OBJECTS += ../suska-iii/buttons.o
	OBJECTS += shell_poll.o
endif

ifeq ($(BOARD),SUSKA_BF)
	OBJECTS += ../suska-iii/ps2int.o
	OBJECTS += ../suska-iii/keyboard.o
	OBJECTS += ../suska-iii/mouse.o
	OBJECTS += ../suska-iii/buttons.o
	OBJECTS += shell_poll.o ../suska-iii/sdrawfile.o
	OBJECTS += ../softuart/softuart.o
	OBJECTS += ../suska-iii/timer_overflow.o
	OBJECTS += ../suska-iii/osd.o
	OBJECTS += ../suska-iii/joystick.o
        OBJECTS += ../config/configshell.o 
endif
ifeq ($(BOARD),SUSKA_B)
	OBJECTS += ../suska-iii/ps2int.o
	OBJECTS += ../suska-iii/keyboard.o
	OBJECTS += ../suska-iii/mouse.o
	OBJECTS += ../suska-iii/buttons.o
	OBJECTS += shell_poll.o
	OBJECTS += ../suska-iii/timer_overflow.o
	OBJECTS += ../suska-iii/joystick.o
endif

CFLAGS += -I.. -I$(CURDIR) -I. -D$(BOARD) -D$(OPTIONAL)

# Additional libraries
#
## Minimalistic printf version
LDFLAGS += -Wl,-u,vfprintf -lprintf_min
#
## Floating point printf version (requires -lm below)
#LDFLAGS +=  -Wl,-u,vfprintf -lprintf_flt
#
## -lm = math library
LIBS = -lm 


# Name of Makefile for make depend
MAKEFILE = Makefile

$(OBJECTS): config.mk


all: $(TARGET).hex $(TARGET).lss $(TARGET).bin
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
	@echo -n "size is: "
	@$(SIZE) -A $(TARGET).hex | grep "\.sec1" | tr -s " " | cut -d" " -f2
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
