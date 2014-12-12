Runs CP/M 2.2 for i8080 under emulation on Linux.

Sources:
- _cpma.cpm_ and _zork1.cpm_ are from Stefan Tramm's nice
[Intel 8080 CPU Emulator](http://www.tramm.li/i8080/)
- CPM22.ASM and CBIOS.ASM were found 
[here](http://maben.homeip.net/static/S100/software/DRI/sourcecode/CPM22/),
and compiled with [asm8080](http://sourceforge.net/projects/asm8080/)
- blank.cpm was created with
```
tr \\000 \\345 </dev/zero | dd  of=foo.cpm count=256256 bs=1
```
- populate images with [cpmtools](http://www.moria.de/~michael/cpmtools/)

Other useful CP/M links:
- http://www.retrotechnology.com/dri/howto_cpm.html
- http://www.seasip.demon.co.uk/Cpm/
- http://www.gaby.de/cpm/
- http://cpuville.com/cpm_on_new_computer.html
- Ben Ryves has a nice [set of posts](http://benryves.com/journal/tags/CP/M) 
describing construction of a modern Z80-based CP/M machine.

