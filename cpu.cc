/*
 * cpu.cc
 */
#ifdef __GNUG__
#pragma implementation
#endif

#include "memory.h"
#include "cpu.h"

CPU::Builder CPU::build;

CPU *CPU::Builder::operator() (const char *n, Memory &m, jmp_buf *e, statfn s) {
	Loader::initfn i = load (n);
	CPU::buildfn build = (CPU::buildfn)i;
	return build (m, e, s);
}

void CPU::debug() { _debug = !_debug; }
