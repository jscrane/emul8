CPPFLAGS=-I.. -DCPU_DEBUG
CXXFLAGS=-g -fno-operator-names
LDLIBS=-L../x86_64-Linux -lemu -ldl

all: z80test tests.me

z80test: z80test.o z80.o ram.o
	g++ -o $@ $+ $(LDLIBS)

%.o: ../%.cc ../%.h
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

tests.me: z80test
	./z80test tests.in > $@

clean:
	rm -f z80test *.o tests.me
