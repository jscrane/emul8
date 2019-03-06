#ifndef _CPU_H
#define _CPU_H

#ifndef _SETJMP_H
#include <setjmp.h>
#endif

#ifdef __GNUG__
#pragma interface
#endif

#ifndef _LOADER_H
#include <loader.h>
#endif

class Memory;

#define O(o, e) case o: e(); break
#define A(o, e, a) case o: e(a); break
#define C(o) case o:
#define D(e) default: e(); break

class CPU {
public:
	virtual void run(unsigned instructions) =0;
	virtual void reset() =0;
	virtual void raise(uint8_t level) =0;
	virtual char *status(char *buf, size_t n, bool hdr) =0;
	virtual ~CPU() {}

	typedef CPU *(*buildfn) (Memory &);

	static class Builder: private Loader {
	public:
		CPU *operator() (const char *, Memory &);
	} build;

	inline bool halted() { return _halted; }
	Memory::address pc() { return PC; }

protected:
	CPU(Memory &mem): _mem(mem), _halted(false) {}
	Memory &_mem;
	Memory::address PC;
	bool _halted;
};
#endif
