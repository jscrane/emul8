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
b36d7fc374cd889b8a100b759fac104e tests.me
$ vi -d tests.expected tests.me
```

Flag Errors
===========

|test	|exp	|act	|op		|error	|
|-------|-------|-------|---------------|-------|
|37_1	|ed	|c5	|scf		| E:5,3	|
|3f	|58	|50	|ccf		| E:3	|
|edba_1	|00	|04	|indr		| G:P	|
