CPPFLAGS=-I..
CXXFLAGS=-g
LDLIBS=-L../x86_64-Linux -lemu -ldl

test: test.o 6502.o
	g++ -o $@ $+ $(LDLIBS)

6502.o: ../6502.cc
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f test test.o 6502.o
