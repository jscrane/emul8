CPPFLAGS:=-I..
CXXFLAGS:=-g
LIBS:=../x86_64-Linux
LDLIBS:=-L$(LIBS) -lemu -ldl

test: test.o
	g++ -o $@ $+ $(LIBS)/ram.so $(LIBS)/i8080.so $(LDLIBS)

%.o: %.cc
	g++ $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -f test test.o
