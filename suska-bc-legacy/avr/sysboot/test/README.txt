This program is used as standalone to test button logic and
to check for correct flash programming (erase/program/read cycle).

UART debugging params are 38400bps 8N1

Usage: start a modem like program e.g. minicom (Linux) and set
the serial line parameters. Next open sysboot-test.c and
set MODE. Program into AVR with the make file (make program).
You might adjust the serial port for the programmer in the 
Makefile too.
