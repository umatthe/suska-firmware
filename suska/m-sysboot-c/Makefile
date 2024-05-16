TARGET = sysboot
BOARD = SUSKA_C


ifeq ($(BOARD),SUSKA_C)
	MCU = atmega16
	F_CPU = 8000000UL
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
ifeq ($(MCU),atmega8)
	AVRDUDE_MCU=m8
endif
ifeq ($(MCU),atmega16)
	AVRDUDE_MCU=m16
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

include $(CURDIR)/bootloader.mk

# include avr-generic makefile configuration
include $(CURDIR)/avr.mk

SRC = $(shell echo *.c)
OBJECTS += sysboot.o 
CFLAGS += -I.. -I$(CURDIR) -I. -D$(BOARD) 

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
