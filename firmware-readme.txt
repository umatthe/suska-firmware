Content:
-------
- Folder suska-bc-legacy : Source for the Suska-III-C Power- and PS2-Ctl and Suska-III-B Legacy Code
- Folder suska           : Source for the Suska-III-B, Suska-III-BF and Suska-III-C-SDC
- Folder bin		     : Binaries for all Boards	
- Folder tools           : Source for crcgenfill, needed to build SDC-Images

General Info:
------------

Suska-III-C:
------------
- System-Controller Atmega16-8MHz
  Lock=0x3F LFuse=0xFF HFuse=0xDF
- PS2-Controller    Atmega16-4MHz
  Lock=0x3F LFuse=0xDF HFuse=0x45
- SDCard-Controller Atmega644-16MHz
  Lock=0x3F LFuse=0xE0 HFuse=0xD4 EFuse=0xFF

Suska-III-B:
------------
- System-Controller Atmega644-8MHz
  Lock=0x3F LFuse=0xE7 HFuse=0xD9 EFuse=0xFF

Suska-III-BF:
-------------
- System-Controller Atmega649a-16MHz  (disable JTAG PF7..PF4 may be used by SW)
  Lock=0x3F LFuse=0xEF HFuse=0xD7 EFuse=0xFF

Suska-IV-B*:
------------
- Same binary as Suska-III-BF

### Important: remove the micro SD card before flashing the AVR! ###

Howto Flash via avrdude:
------------------------
Suska-III-BF: avrdude -c avrispmkII -p m649 -U f:w:bin/suska-bf/system-bf.hex -P usb -F
Suska-III-B:  avrdude -c avrispmkII -p m644 -U f:w:bin/suska-iii-b/system-b.hex -P usb
Suska-III-C:  avrdude -c avrispmkII -p m644 -U f:w:bin/suska-iii-C/sdc-c.hex -P usb
Suska-IV-B*:  avrdude -c avrispmkII -p m649 -U f:w:bin/suska-bf/system-bf.hex -P usb -F
