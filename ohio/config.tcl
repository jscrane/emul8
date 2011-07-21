#
# c1s.tcl -- ohio challenger hardware configuration
#
processor 6502
device ram 0x0000 0x2000

device rom 0xa000 0x0800 "ohio/roms/basic-1"
device rom 0xa800 0x0800 "ohio/roms/basic-2"
device rom 0xb000 0x0800 "ohio/roms/basic-3"
device rom 0xb800 0x0800 "ohio/roms/basic-4"
device rom 0xf800 0x0800 "ohio/roms/monitor"

device display 0xd000 32 32 "uk101/chars"	

device kbduk_tk 0xdf00
device tapeuk_tk 0xf000 0x0100
