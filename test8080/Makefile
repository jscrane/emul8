CPPFLAGS=-I.. -DCPU_DEBUG
CXXFLAGS=-g
LDLIBS=-L../i686-Linux -lemu -ldl

test: test.o i8080.o ram.o
	g++ -o $@ $+ $(LDLIBS)

%.o: ../%.cc
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f test test.o i8080.o
