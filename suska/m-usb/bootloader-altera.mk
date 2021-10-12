DEVID = 0x0000000a
SWVERSIONMAJOR = 0 
SWVERSIONMINOR = 0
BOOTLDRSIZE = 0x800
BOOTLDRINFOSTART = 0xF7F8

CRCGEN = crcgenfill

LDFLAGS	+= -Wl,--section-start=.bootldrinfo=$(BOOTLDRINFOSTART)

CDEFS += -DDEVID=$(DEVID)
CDEFS += -DSWVERSIONMAJOR=$(SWVERSIONMAJOR)
CDEFS += -DSWVERSIONMINOR=$(SWVERSIONMINOR)

CFLAGS += $(CDEFS)

%.bin: %
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O binary -R .eeprom $< $@
	@echo
	$(CRCGEN) $@ `$(SIZE) -A $(TARGET).hex | grep "\.sec1" | tr -s " " | cut -d" " -f2`
	@echo
	bin2hex $(TARGET).bin >$(TARGET)-altera.hex
	cp $(TARGET)-altera.hex ../../../Configware/rtl/vhdl/UDOS/avrcore/hdl-core-mega-xmega/suska/main.hex
