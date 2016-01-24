/*
 * cpu.cc
 */
#ifdef __GNUG__
#pragma implementation
#endif
#include <stddef.h>

#include "memory.h"
#include "cpu.h"

CPU::Builder CPU::build;

CPU *CPU::Builder::operator()(const char *n, Memory &m) {
	Loader::initfn i = load(n);
	CPU::buildfn build = (CPU::buildfn)i;
	return build(m);
}
