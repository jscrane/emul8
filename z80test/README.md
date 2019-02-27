z80test
-------

A test harness for a z80 processor emulator; based on, and using tests from,
the excellent [FUSE](http://fuse-emulator.sourceforge.net/).

```bash
$ export LD_LIBRARY_PATH=../x86_64-Linux
$ make
g++ -g -fno-operator-names -I.. -DCPU_DEBUG  -c -o z80test.o z80test.cc
g++ -g -fno-operator-names -I.. -DCPU_DEBUG -o z80.o -c ../z80.cc
g++ -g -fno-operator-names -I.. -DCPU_DEBUG -o ram.o -c ../ram.cc
g++ -o z80test z80test.o z80.o ram.o -L../x86_64-Linux -lemu -ldl
./z80test tests.in > tests.me
md5sum tests.me
57da905261bbe344436edd184176731c  tests.me
$ vi -d tests.expected tests.me
```

Flag Errors
===========

|test	|exp	|act	|op	|
|-------|-------|-------|--------
|37_1	|ed	|c5	|scf	|
|3f	|58	|50	|ccf	|
|cb46_2	|38	|10	|bit 0,(hl)	|
|cb46_3	|38	|10	|
|cb46_4	|18	|10	|
|cb46_4	|30	|10	|
|cb4e	|10	|18	|bit 1,(hl)	|
|cb5e	|10	|38	|bit 3,(hl)	|
|cb6e	|10	|30	|bit 5,(hl)	|
|cb76	|54	|5c	|bit 6,(hl)	|
|eda0	|e5	|c5	|ldi	|
|eda1	|0f	|2f	|cpi	|
|eda2	|a4	|84	|ini	|
|eda9	|bf	|9f	|cpd	|
|edb0	|00	|20	|ldir	|
|edb0_2	|20	|00	|
|edb1_2	|87	|a7	|
|edb8	|69	|49	|lddr	|
|edb8_1	|20	|00	|
|edb8_2	|00	|20	|
|edb9	|0b	|2b	|cpdr	|
|edb9_1	|0b	|2b	|
|edb9_2	|a7	|87	|
|edba_1	|00	|04	|
