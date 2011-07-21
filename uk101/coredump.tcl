#
# uk101.tcl -- uk101 hardware configuration
#
processor 6502
device ram 0x0000 0x4000

# does toolkit-ii reside at addr 8000, 9000, or 9800?
#
device rom 0x8000 0x0800 "uk101/roms/toolkit-ii"
device rom 0x8800 0x0800 "uk101/roms/encoder"
device rom 0xa000 0x2000 "uk101/roms/basic"
device rom 0xc000 0x0800 "uk101/roms/exmon"
device rom 0xf800 0x0800 "uk101/roms/bambleweeny"

device display 0xd000 64 32 "uk101/chars"	

device kbduk_tk 0xdf00
device tapeuk_tk 0xf000 0x0100

# device via 0xf700 0x0100
