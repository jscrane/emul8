#ifndef _RAM_H
#define _RAM_H

#ifdef __GNUG__
#pragma interface
#endif

class ram: public Memory::Device {
public:
	virtual void operator= (byte c) { _mem[_acc] = c; }
	virtual operator byte () { return _mem[_acc]; }

	ram (int bytes): Memory::Device(bytes/Memory::page_size), 
		_mem(new byte[bytes]) {}
	~ram () { delete[] _mem; }

	const char *name () const { return "ram"; }

	class Builder: public Memory::Device::Builder {
	public:
		bool recognises (const char *s);
		bool build (Memory *, int, const char **);
	};
private:
	byte *_mem;
};
#endif
