/*
 * cpu.h
 */
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

class CPU {
public:
	virtual void reset () =0;
	virtual void run (unsigned instructions) =0;
	virtual void raise (int level) =0;
	virtual char *status () =0;

	typedef void (*statfn) (const char *, ...);
	typedef CPU *(*buildfn) (Memory &, jmp_buf *, statfn);

	static class Builder: private Loader {
	public:
		CPU *operator() (const char *, Memory &, jmp_buf *, statfn);
	} build;

	Memory::address pc() { return PC; }

	void debug();

protected:
	Memory &_mem;
	CPU (Memory &m, jmp_buf *e, statfn s): _mem(m), _err(e), _status(s) {}
	Memory::address PC;
	jmp_buf *_err;
	statfn _status;
	bool _debug;
};
#endif
