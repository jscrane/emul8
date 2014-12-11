CPPFLAGS=-I..
CPPFLAGS=-I.. -DCPU_DEBUG -g
#CXXFLAGS=-g -O3
LDLIBS=-L../i686-Linux -lemu -ldl

cpm: cpm.o i8080.o ram.o
	g++ -o $@ $+ $(LDLIBS)

%.o: ../%.cc
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f cpm *.o
