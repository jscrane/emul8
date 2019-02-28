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

|test	|exp	|act	|op		|error	|
|-------|-------|-------|---------------|-------|
|37_1	|ed	|c5	|scf		| E:5,3	|
|3f	|58	|50	|ccf		| E:3	|
|cb46_2	|38	|10	|bit 0,(hl)	| E:5,3 |
|cb46_3	|38	|10	|		| E:5,3	|
|cb46_4	|18	|10	|		| E:3	|
|cb46_4	|30	|10	|		| E:5	|
|cb4e	|10	|18	|bit 1,(hl)	| G:3	|
|cb5e	|10	|38	|bit 3,(hl)	| G:5,3	|
|cb6e	|10	|30	|bit 5,(hl)	| G:5	|
|cb76	|54	|5c	|bit 6,(hl)	| G:3	|
|eda0	|e5	|c5	|ldi		| E:5	|
|eda1	|0f	|2f	|cpi		| G:5	|
|eda2	|a4	|84	|ini		| E:5	|
|eda9	|bf	|9f	|cpd		| E:5	|
|edb0	|00	|20	|ldir		| G:5	|
|edb0_2	|20	|00	|		| E:5	|
|edb1_2	|87	|a7	|		| G:5	|
|edb8	|69	|49	|lddr		| E:5	|
|edb8_1	|20	|00	|		| E:5	|
|edb8_2	|00	|20	|		| G:5	|
|edb9	|0b	|2b	|cpdr		| G:5	|
|edb9_1	|0b	|2b	|		| G:5	|
|edb9_2	|a7	|87	|		| E:5	|
|edba_1	|00	|04	|indr		| G:P	|
