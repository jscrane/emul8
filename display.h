/*
 * display.h -- display device abstraction
 */
#ifndef _DISPLAY_H
#define _DISPLAY_H

#ifndef _MEMORY_H
#include "memory.h"
#endif

class display: public Memory::Device {
public:
	virtual void operator= (byte c) { _mem[_acc] = c; }
	virtual operator byte () { return _mem[_acc]; }

	display (int r, int c): Memory::Device((r*c)/Memory::page_size),
		_mem(new byte[r*c]) {}
	virtual ~display () { delete[] _mem; }
private:
	byte *_mem;
};

#endif
