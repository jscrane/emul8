/*
 * kdbuk_tk -- Tk/UK101 keyboard device
 */
#include <tcl.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"

class kbd: public Memory::Device {
public:
	void operator= (byte);
	operator byte ();
	const char *name () const { return "kbduk_tk"; }

	void up (unsigned);
	void down (unsigned);
	kbd (): Memory::Device(1), _last(0), _delay(128) {
		for (int i=8; i--; )
			_rows[i] = 0;
	}
	void delay (int d) { _delay=d; }
	int delay () const { return _delay; }

	class Builder: public Memory::Device::Builder {
	public:
		bool recognises (const char *s) { 
			return !strcmp (s, "kbduk_tk"); 
		}
		bool build (Memory *, int, const char **);
		Builder (): _k(0) {}
	private:
		kbd *_k;
	};
private:
	byte _rows[8];
	byte _last;
	bool _map (unsigned, unsigned short &);
	void _set (byte, bool);
	void _reset (byte);
	int _delay;
};

void kbd::operator= (byte row) {
	_last = row;
	for (int i=0; i<_delay; i++)
		Tcl_DoOneEvent (TCL_ALL_EVENTS | TCL_DONT_WAIT);
}

kbd::operator byte () {
	int r = 255-_last;
	byte pattern = 0;
	for (int i=8; i--; r>>=1) 
		if (r & 1)
			pattern |= _rows[7-i];
	return pattern ^ 0xff;
}

static unsigned short keymap[128] = {
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0x0014, 0x0277, 0x0276, 0x0275, 0x0274, 0x0273, 0x0272, 0x0271,
	0x0267, 0x0266, 0x0264, 0x0112, 0x0021, 0x0063, 0x0057, 0x0013,
	0x0065, 0x0077, 0x0076, 0x0075, 0x0074, 0x0073, 0x0072, 0x0071,
	0x0067, 0x0066, 0x0064, 0x0012, 0x0221, 0x0263, 0x0257, 0x0213,
	0x0211, 0x0016, 0x0024, 0x0026, 0x0036, 0x0046, 0x0035, 0x0034,
	0x0033, 0x0041, 0x0032, 0x0031, 0x0056, 0x0022, 0x0023, 0x0055,
	0x0011, 0x0017, 0x0045, 0x0037, 0x0044, 0x0042, 0x0025, 0x0047,
	0x0027, 0x0043, 0x0015, 0x0231, 0xffff, 0x0222, 0x0223, 0xffff,
	0x0256, 0x0016, 0x0024, 0x0026, 0x0036, 0x0046, 0x0035, 0x0034,
	0x0033, 0x0041, 0x0032, 0x0031, 0x0056, 0x0022, 0x0023, 0x0055,
	0x0011, 0x0017, 0x0045, 0x0037, 0x0044, 0x0042, 0x0025, 0x0047,
	0x0027, 0x0043, 0x0015, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
};

bool kbd::_map (unsigned key, unsigned short &map) {
	switch (key) {
	case 0xff08:	// backspace
		map = 0x62;
		break;
	case 0xffe3:	// l-ctrl
		map = 0x06;
		break;
	case 0xffe1:	// l-shift
		map = 0x02;
		break;
	case 0xffe2:	// r-shift
		map = 0x01;
		break;
	case 0xffe5:	// shift-lock
		map = 0x00;
		return false;
	case 0xff0d:	// cr
		map = 0x53;
		break;
	case 0xff52:	// up-arrow
		map = 0x54;
		break;
	case 0xff54:	// down-arrow (linefeed)
		map = 0x0232;
		break;
	default:
		map = key<128? keymap[key]: 0xff;
		break;
	}
	return true;
}

void kbd::_reset (byte k) {
	byte &r = _rows[(k & 0xf0) >> 4];
	byte c = 1 << (k & 0x0f);
	r &= ~c;
}

void kbd::up (unsigned key) {
	unsigned short k;
	if (_map (key, k) && k != 0xffff) {
		if (k > 0xff) _reset (k/0xff);
		_reset (k&0xff);
	}
}

void kbd::_set (byte k, bool lock) {
	byte &r = _rows[(k & 0xf0) >> 4];
	byte c = 1 << (k & 0x0f);
	if (lock)
		r ^= c;
	else
		r |= c;
}

void kbd::down (unsigned key) {
	unsigned short k;
	bool lock = !_map (key, k);
	if (k != 0xffff) {
		if (k > 0xff) _set (k/0xff, lock);
		_set (k&0xff, lock);
	}
}

// Commands added to the Tcl Interpreter
//
extern "C" int KeyUp (ClientData d, Tcl_Interp *, int, const char **av) {
	unsigned keysym;
	sscanf (av[1], "%d", &keysym);
	kbd *k = (kbd *)d;
	k->up (keysym);
	return TCL_OK;
}

extern "C" int KeyDown (ClientData d, Tcl_Interp *, int, const char **av) {
	unsigned keysym;
	sscanf (av[1], "%d", &keysym);
	kbd *k = (kbd *)d;
	k->down (keysym);
	return TCL_OK;
}

extern "C" int KeyDelay (ClientData d, Tcl_Interp *i, int ac, const char **av) {
	kbd *k = (kbd *)d;
	if (ac == 2) {
		int d;
		sscanf (av[1], "%d", &d);
		k->delay (d);
	} else if (ac == 1) {
		char buf[16];
		sprintf (buf, "%d", k->delay ());
		Tcl_SetResult (i, buf, TCL_VOLATILE);
	} else {
		Tcl_SetResult (i, "Wrong # args", TCL_STATIC);
		return TCL_ERROR;
	}
	return TCL_OK;
}

extern "C" Tcl_Interp *getInterp ();

bool kbd::Builder::build (Memory *m, int ac, const char **av) {
	if (ac != 1 || _k)
		return false;
	unsigned base;
	sscanf (av[0], "%x", &base);
	_k = new kbd;
	m->put (*_k, base);
	Tcl_Interp *i = getInterp ();
	Tcl_CreateCommand (i, "key_up", KeyUp, _k, 0);
	Tcl_CreateCommand (i, "key_down", KeyDown, _k, 0);
	Tcl_CreateCommand (i, "key_delay", KeyDelay, _k, 0);
	if (TCL_OK == Tcl_EvalFile (i, TCL"kbd.tcl"))
		return true;
//	fprintf (stderr, "Tcl_Eval returns: %s\n", i->result);
	return false;
}

// Module initialisation
//
extern "C" void *init_kbduk_tk () {
	return (Memory::Device::Builder *)new kbd::Builder;
}
