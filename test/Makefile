CPPFLAGS=-I..
CXXFLAGS=-g

test: test.o 6502.o
	g++ -g -o test test.o 6502.o -L../i686-Linux -lemu -ldl

6502.o: ../6502.cc ../6502.h
	g++ -g -I.. -DDEBUG=1 -c ../6502.cc -o 6502.o

clean:
	rm -f test test.o 6502.o
