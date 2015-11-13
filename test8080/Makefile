CPPFLAGS=-I..
#CPPFLAGS=-I.. -DCPU_DEBUG
CXXFLAGS=-g -O3
LDLIBS=-L../x86_64-Linux -lemu -ldl

test: test.o i8080.o ram.o
	g++ -o $@ $+ $(LDLIBS)

%.o: ../%.cc
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f test test.o i8080.o
