This Atmel firmware manages the upload of an Atari OS Image
(e.g. EmuTOS or Atari OS) through the Suska FPGA into the Flash
PROMS of Suska. Before this works, it is necessary to have
the FPGA configware programmed into the FPGA boot device.

On the PC side there is a Linux program (suska-flasher) that
sends the OS Image to the Atmel. The program connects 
the PC via a serial USB interface to the Atmel.

