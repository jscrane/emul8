/*
 * display.cc
 */
#include <stdio.h>
#include <string.h>
#include <tcl.h>
#include <tk.h>
#include "display.h"

class TkDisplay: public display {
public:
	TkDisplay (int r, int c, char *s, Tcl_Interp *i): display(r, c), 
	_r(r), _c(c), _s(strdup (s)), _i(i) {
		Tcl_EvalFile (_i, TCL"display.tcl");
//		if (TCL_OK != Tcl_EvalFile (_i, TCL"display.tcl"))
//			puts (_i->result);
		char b[80];
		sprintf (b, "display_init %d %d 8 8 %s", c, r, s);
		Tcl_Eval (_i, b);
//		if (TCL_OK != Tcl_Eval (_i, b))
//			puts (_i->result);
	}
	~TkDisplay () {
		Tcl_Eval (_i, "display_fini");
//		if (TCL_OK != Tcl_Eval (_i, "display_fini"))
//			puts (_i->result);
	}
	void operator= (byte c) {
		if (c != (byte)*this) {
			char b[80];
			sprintf (b, "display_at %d %d", _acc, c);
			Tcl_Eval (_i, b);
//			if (TCL_OK != Tcl_Eval (_i, b))
//				puts (_i->result);
			display::operator= (c);
		}
	}
	const char *name () const {
		static char b[80];
		sprintf (b, "display %d %d %s", _c, _r, _s);
		return b;
	}

	class Builder: public Memory::Device::Builder {
	public:
		bool recognises (const char *s) {
			return !strcmp (s, "display");
		}
		bool build (Memory *, int, const char **);
	};

private:
	int _r, _c;
	char *_s;
	Tcl_Interp *_i;
};

extern "C" Tcl_Interp *getInterp ();

bool TkDisplay::Builder::build (Memory *m, int ac, const char **av) {
	if (ac != 4)
		return false;
	unsigned base, row, col;
	sscanf (av[2], "%d", &row);
	sscanf (av[1], "%d", &col);
	TkDisplay *d = new TkDisplay(row, col, (char *)av[3], getInterp ());
	sscanf (av[0], "%x", &base);
	m->put (*d, base);
	return true;
}

extern "C" void *init_display () {
	return (Memory::Device::Builder *)new TkDisplay::Builder;
}
