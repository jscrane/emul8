/*
 * tapeuk_tk.cc -- UK101/Tk Tape Interface
 */
#include <tcl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "acia.h"

class tape: public acia {
public:
	const char *name () const { return "tapeuk_tk"; }
	void operator= (byte);
	operator byte ();
	
	tape (): _file(strdup ("session.log")), _fd(-1) {}
	~tape () { if (_file) { free (_file); _file=0; } }

	char *file () const { return _file; }
	void file (const char *f) { 
		delete _file; 
		if (_fd != -1)
			close (_fd);
		_file = strdup (f); 
		// if we can read it, it's for loading, else saving
		//	
		_fd = open (f, ::access (f, R_OK)? O_CREAT | O_WRONLY: O_RDONLY, 0666);
		if (_fd < 0)
			perror ("open");
		_pos = _len = sizeof(_buffer);
	}
	void rewind () { lseek (_fd, 0, SEEK_SET); }
	bool isOpen () { return _fd != -1; }

	class Builder: public Memory::Device::Builder {
	public:
		bool recognises (const char *s) {
			return !strcmp (s, "tapeuk_tk");
		}
		bool build (Memory *, int, const char **);
	};
private:
	char *_file;
	byte _buffer[1024];
	int _fd, _pos, _len;
};

void tape::operator= (byte b) {
	if ((_acc & 1) && 0 > ::write (_fd, &b, 1))
		perror ("write");
}

tape::operator byte () {
	if (_acc & 1)			// read data
		return _buffer[_pos++];
	if (_pos >= _len) {		// read status
		_pos = 0;
		_len = ::read (_fd, _buffer, sizeof(_buffer));
		if (!_len)
			return 0;	// nothing there
	}
	return acia::rdrf || acia::tdre;
}

extern "C" int Tape (ClientData d, Tcl_Interp *i, int ac, const char **av) {
	tape *t = (tape *)d;
	if (!strcmp (av[1], "rewind")) {
		Tcl_SetResult (i, (char *)(t->isOpen()? "normal":"disabled"), TCL_STATIC);
		if (t->isOpen ()) t->rewind ();
	} else if (!strcmp (av[1], "filename")) {
		if (ac==3)
			t->file (av[2]);
		else if (ac==2)
			Tcl_SetResult (i, t->file (), TCL_STATIC);
		else {
			Tcl_SetResult (i, "Wrong # args", TCL_STATIC);
			return TCL_ERROR;
		}
	} else {
		Tcl_SetResult (i, "Unknown tape request", TCL_STATIC);
		return TCL_ERROR;
	}
	return TCL_OK;
}

extern "C" Tcl_Interp *getInterp ();

bool tape::Builder::build (Memory *m, int ac, const char **av) {
	unsigned base;
	sscanf (av[0], "%x", &base);
	tape *t = new tape;
	m->put (*t, base);
	Tcl_Interp *i = getInterp ();
	Tcl_CreateCommand (i, "tape", Tape, t, 0);
	if (TCL_OK == Tcl_EvalFile (i, TCL"tape.tcl"))
		return true;
	fprintf (stderr, "Tcl_Eval returns: %s\n", i->result);
	return false;
}

// Module initialisation
//
extern "C" void *init_tapeuk_tk () {
	return (Memory::Device::Builder *)new tape::Builder;
}
