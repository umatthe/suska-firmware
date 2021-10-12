Content:
-------
- Folder suska-bc-legacy : Source for the Suska-III-C Power- and PS2-Ctl and Suska-III-B Legacy Code
- Folder suska           : Source for the Suska-III-B, Suska-III-BF and Suska-III-C-SDC
- Folder bin		 : Binaries for all Boards	

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
- System-Controller Atmega649a-16MHz
  Lock=0x3F LFuse=0xEF HFuse=0x97 EFuse=0xFF

Howto Flash via avrdude:
------------------------
Suska-BF: avrdude -c avrispmkII -p m649 -U f:w:system-bf.hex -F
Suska-B:  avrdude -c avrispmkII -p m644 -U f:w:system-b.hex
Suska-C: avrdude -c avrispmkII -p m644 -U f:w:sdc-c.hex

