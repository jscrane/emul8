#
# uk101.tcl -- original uk101 hardware configuration
#
processor 6502
device ram 0x0000 0x2000

device rom 0xa000 0x2000 "uk101/roms/basic"
device rom 0xf800 0x0800 "uk101/roms/mon02"

device display 0xd000 64 32 "uk101/chars"	

device kbduk_tk 0xdf00
device tapeuk_tk 0xf000 0x0100
