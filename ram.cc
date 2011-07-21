#include <stdio.h>
#include <string.h>
#include "memory.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "ram.h"

bool ram::Builder::recognises (const char *s) { return !strcmp (s, "ram"); }

bool ram::Builder::build (Memory *m, int ac, const char **av) {
	if (ac != 2)
		return false;
	unsigned base, size;
	sscanf (av[1], "%x", &size);
	ram *r = new ram(size);
	sscanf (av[0], "%x", &base);
	m->put (*r, base);
	return true;
}

extern "C" void *init_ram () {
	return (Memory::Device::Builder *)new ram::Builder;
}
