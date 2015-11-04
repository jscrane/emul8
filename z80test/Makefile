CPPFLAGS=-I..
CPPFLAGS=-I.. -DCPU_DEBUG
CXXFLAGS=-g -fno-operator-names
LDLIBS=-L../x86_64-Linux -lemu -ldl

all: z80test

z80test: z80test.o z80.o ram.o
	g++ -o $@ $+ $(LDLIBS)

%.o: ../%.cc
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f z80test *.o
