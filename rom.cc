#include <tcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

class rom: public Memory::Device {
public:
	virtual void operator= (byte) {}
	virtual operator byte () { return _mem[_acc]; }

	rom (const char *filename, int pages);
	~rom () { free (_name); }

	const char *name () const { return _name; }

	class Builder: public Memory::Device::Builder {
	public:
		Builder ();
		~Builder ();
		char *name () const { return _prom; }
		void dump (const char *, long, long);

		bool recognises (const char *s) { return !strcmp (s, "rom"); }
		bool build (Memory *, int, const char **);
	private:
		char *_prom;
		Memory *_memory;
	};
private:
	byte *_mem;
	char *_name;
};

rom::rom (const char *filename, int b): Memory::Device(b/Memory::page_size), _mem(new byte[b]) {
	FILE *fp = fopen (filename, "r");
	for (int i=0; i<b; i++)
		fscanf (fp, "%02x", _mem+i);
	fclose (fp);

	_name = (char *)malloc (64);
	strcpy (_name, "rom ");
	strcat (_name, filename);
}

bool rom::Builder::build (Memory *m, int ac, const char **av) {
	_memory = m;
	if (ac == 3) {
		unsigned base, size;
		sscanf (av[1], "%x", &size);
		rom *r = new rom(av[2], size);
		sscanf (av[0], "%x", &base);
		m->put (*r, base);
		return true;
	}
	return false;
}

void rom::Builder::dump (const char *p, long fp, long tp) {
	free (_prom);
	_prom = strdup (p);
	FILE *f = fopen (_prom, "w");
	for (long from=fp*256, to=tp*256; from < to;) {
		for (int i=32; i--; )
			fprintf (f, "%02x", (byte)(*_memory)[from++]);
		fprintf (f, "\n");
	}
	fclose (f);
}

extern "C" int Prom (ClientData d, Tcl_Interp *i, int ac, const char **av) {
	rom::Builder *b = (rom::Builder *)d;
	if (ac==1)
		Tcl_SetResult (i, b->name (), TCL_STATIC);
	else if (ac==4)
		b->dump (av[1], strtol (av[2], 0, 16), strtol (av[3], 0, 16));
	else {
		Tcl_SetResult (i, "Wrong # args", TCL_STATIC);
		return TCL_ERROR;
	}
	return TCL_OK;
}

extern "C" Tcl_Interp *getInterp ();

rom::Builder::Builder (): _prom(strdup ("new_prom")) {
	Tcl_Interp *i = GetInterp ();
	if (TCL_OK == Tcl_EvalFile (i, TCL"prom.tcl"))
		Tcl_CreateCommand (i, "prom", Prom, this, 0);
//	else
//		fprintf (stderr, "Tcl_Eval returns: %s\n", i->result);
}

rom::Builder::~Builder () {
	Tcl_DeleteCommand (getInterp (), "prom");
	if (_prom) { free (_prom); _prom=0; }
}

extern "C" void *init_rom () {
	return (Memory::Device::Builder *)new rom::Builder;
}
