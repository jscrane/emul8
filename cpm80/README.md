CP/M 2.2 for i8080 under emulation on Linux.

```bash
$ make
$ export LD_LIBRARY_PATH=../x86_64-Linux
$ ./cpm cpma.com zork1.com
a>dir
A: DUMP     COM : SDIR     COM : SUBMIT   COM : ED       COM
A: STAT     COM : BYE      COM : RMAC     COM : CREF80   COM
A: DDTZ     COM : L80      COM : M80      COM : SID      COM
A: WM       COM : WM       HLP : ZSID     COM : MAC      COM
A: TRACE    UTL : HIST     UTL : LIB80    COM : M        SUB
A: DDT      COM : CLS      COM : MOVCPM   COM : ASM      COM
A: LOAD     COM : XSUB     COM : HELLO    ASM : PIP      COM
A: SYSGEN   COM : REW      ASM : HELLO    COM : REW      COM
a>b:
b>zork1
ZORK I: The Great Underground Empire
Copyright (c) 1981, 1982, 1983 Infocom, Inc. All rights
reserved.
ZORK is a registered trademark of Infocom, Inc.
Revision 88 / Serial number 840726

West of House
You are standing in an open field west of a white house, with
a boarded front door.
There is a small mailbox here.

>
```

Sources:
- CPM disk images are from Stefan Tramm's fine
[Intel 8080 CPU Emulator](http://www.tramm.li/i8080/)
- CPM22.ASM and CBIOS.ASM were found 
[here](http://maben.homeip.net/static/S100/software/DRI/sourcecode/CPM22/),
and compiled with [asm8080](http://sourceforge.net/projects/asm8080/)
- create new images with ```make blank``` and populate with
[cpmtools](http://www.moria.de/~michael/cpmtools/)

Other useful CP/M links:
- http://www.retrotechnology.com/dri/howto_cpm.html
- http://www.seasip.demon.co.uk/Cpm/
- http://www.gaby.de/cpm/
- http://cpuville.com/cpm_on_new_computer.html
- Ben Ryves has a nice [set of posts](http://benryves.com/journal/tags/CP/M) 
describing construction of a modern Z80-based CP/M machine.

