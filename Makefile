#
# Makefile for 8-bit emulator
#
MACHINE = $(shell uname -m)
OS	= $(shell uname -s)
TARGET	= $(MACHINE)-$(OS)

SOURCES	= chainbase.cc chainable.cc memory.cc cpu.cc loader.cc
HEADERS	= chainbase.h chainable.h chain.h memory.h cpu.h loader.h
OBJECTS	= $(foreach f, $(SOURCES), $(TARGET)/$(f:.cc=.o))
LIBRARY	= libemu
DYLIB	= $(TARGET)/$(LIBRARY).so
LIBRARIES = $(DYLIB)
MODSRC	= tk-emul8.cc 6502.cc ram.cc rom.cc display.cc kbduk_tk.cc tapeuk_tk.cc
MODHDR	= 6502.h ram.h display.h rom.h acia.h
MODOBJ	= $(foreach m, $(MODSRC), $(TARGET)/$(m:.cc=.so))
TCLDIR	= tcllib/
TCLHDRS = /usr/include/tcl8.5
CONFIGS	= uk101
INFO	= TO-DO BUGS CREDITS COPYING README

CC	= g++
LD	= g++
CPPFLAGS= -I. -I$(TCLHDRS) -DTCL=\"$(TCLDIR)\" -DPIC
CXXFLAGS= -O6 -fPIC -fomit-frame-pointer

ifdef $(DEBUG)
CPPFLAGS+= -DDEBUG
endif

all: .depend $(TARGET) $(LIBRARIES) $(MODOBJ)

$(TARGET):
	mkdir $@

$(TARGET)/%.o: %.cc
	 $(CC) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(TARGET)/%.so: $(TARGET)/%.o
	$(CC) -shared -nostartfiles $< -L$(TARGET) -lemu -o $@

$(DYLIB): $(OBJECTS)
	$(LD) -shared -Wl,--export-dynamic $^ -o $@

clean:
	rm -r $(MACHINE)-$(OS) .depend

CORE	= Makefile tk-emul8 $(INFO) $(SOURCES) $(HEADERS) $(MODSRC) $(MODHDR) $(TCLDIR)

release:
	tar zcf emul8.tar.gz $(CORE)

confs:
	$(foreach c, $(CONFIGS), tar zcf $(c).tar.gz $(c))

tar:
	tar zcf emul8.tar.gz $(CORE) $(CONFIGS)

.depend: $(SOURCES) $(HEADERS) $(MODSRC) $(MODHDR)
	$(CC) -MM -MG $(CPPFLAGS) $(SOURCES) > $@
	$(CC) -MM -MG $(CPPFLAGS) $(MODSRC) | sed -e "s?^\(.*\.o\)?$(TARGET)/\1?" >> $@

-include .depend
