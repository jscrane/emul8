CPPFLAGS=-I.. -fno-operator-names
#CPPFLAGS+=-DCPU_DEBUG -g
CXXFLAGS=-g -O3
LDLIBS=-L../x86_64-Linux -lemu -ldl -lcurses

all: cpm z80cpm CPM22.bin CBIOS.bin

cpm: cpm.o i8080.o ram.o
	g++ -o $@ $+ $(LDLIBS)

z80cpm: z80cpm.o z80.o ram.o
	g++ -o $@ $+ $(LDLIBS)

%.bin: %.ASM
	asm8080 $< -l

%.o: ../%.cc
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f cpm *.o *.bin *.lst *.hex

blank:
	tr \\000 \\345 </dev/zero | dd  of=blank.cpm count=256256 bs=1
	
