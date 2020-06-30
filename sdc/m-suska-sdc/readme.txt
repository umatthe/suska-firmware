Installieren mit AVRISP MK2
---------------------------
avrdude -c avrispmkII -p m644 -U f:w:main.hex 

Beim Suska-BF Board ist "force" -F nötig:
avrdude -c avrispmkII -p m644 -U f:w:main.hex -P usb -F
