TARGET = main
# export current directory to use in sub-makefiles
export CURDIR

# programs
CC = m68k-atari-mint-gcc
OBJCOPY = m68k-atari-mint-objcopy
OBJDUMP = m68k-atari-mint-objdump
STRIP = m68k-atari-mint-strip
AS = m68k-atari-mint-as
CP = cp
RM = rm -f

-include $(CURDIR)/config.mk


# flags for the compiler
CFLAGS += -O3 -std=gnu99

SRC = $(shell echo *.c)

OBJECTS += ../misc/atoi.o
OBJECTS += mmc.o mmc-hw.o softspi.o diskio.o
OBJECTS += ../tff/ff.o  ../tff/getfattime.o
#OBJECTS += ../tff/ccsbcs.o

OBJECTS += main.o


CFLAGS += -I$(CURDIR) 

# Additional libraries
#
#LDFLAGS +=  -Wl,-u,vfprintf -lprintf_flt
#
#LIBS = -lm 


# Name of Makefile for make depend
MAKEFILE = Makefile

$(OBJECTS): config.mk


all: $(TARGET).tos 
	$(STRIP) $(TARGET).tos
	@echo "==============================="
	@echo "$(TARGET) compiled for: $(MCU)"
#	@echo -n "size is: "
#	@$(SIZE) -A $(TARGET).hex | grep "\.sec1" | tr -s " " | cut -d" " -f2
	@echo "==============================="

$(TARGET).tos: $(OBJECTS) $(TARGET).o
	$(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET).tos


# subdir magic
%/% %/%.o %/%.hex %/all %/depend %/install %/clean:
	$(MAKE) -C $(@D) -e $(@F)


clean:
	$(RM) *.tos 

clean: clean-$(TARGET) 

clean-$(TARGET):
	rm -f $(TARGET)
	rm -f $(OBJECTS)
