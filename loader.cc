/*
 * loader.cc -- shared object loader
 */
#include <dlfcn.h>
#include <stdio.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "loader.h"

Loader::initfn Loader::load (const char *name) {
	char buf[64];
	sprintf (buf, "%s.so", name);
	void *handle = dlopen (buf, RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		fprintf (stderr, "Loader: loading %s: %s\n", buf, dlerror ());
		return 0;
	}
	sprintf (buf, "init_%s", name);
	initfn dev_init = (initfn)dlsym (handle, buf);
	if (!dev_init) {
		fprintf (stderr, "Loader: resolving %s: %s\n", buf, dlerror ());
		return 0;
	}
	return dev_init;
}
