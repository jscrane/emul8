#ifdef __GNUG__
#pragma implementation
#endif

#include "memory.h"

void Memory::put (Device &dev, address b) {
	Device **d = _pages + b/page_size, **dp = d, *odev = *d;
	int size = dev.pages (), end = b+size, osize = odev->pages (), i, j;
	for (i=size, j=osize; i--; ) {
		*dp++ = &dev;
/* FIXME!
		if (!--j) {
			delete odev;
			odev = *dp;
			j = odev->pages ();
		}
*/
	}
/* FIXME!
	if (j) {
		do {
			Null *n = new Null;
			n->base (end);
			*dp++ = n;
			end += page_size;
		} while (--j);
		delete odev;
	}
*/
	dev.base (b);
}

Memory::Memory (int pages): _pages(new Device *[pages]) {
	int p = 0;
	for (Device **d=_pages; pages--; p+=page_size) {
		Null *n = new Null;
		n->base (p);
		*d++ = n;
	}
	devices.append (new Memory::Device::Loader (devices));
}

Memory::Iter Memory::Iter::operator++ (int) {
	Iter tmp = *this;
	Memory::address a = _curr->base () + Memory::page_size*_curr->pages ();
	_curr = (a > _curr->base ())? _mem->get (a): 0;
	return tmp;
}

Memory::Iter &Memory::Iter::operator-- () {
	Memory::address a = _curr->base ();
	_curr = a? _mem->get (a - Memory::page_size): 0;
	return *this;
}

bool Memory::Device::Loader::recognises (const char *name) {
	Loader::initfn dev_init = load (name);
	_new = (Memory::Device::Builder *)dev_init ();
	_bldrs.prepend (_new);
	return _new->recognises (name);
}

bool Memory::Device::Loader::build (Memory *m, int ac, const char **av) {
	return _new->build (m, ac, av);
}
