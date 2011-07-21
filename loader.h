/*
 * loader.h -- shared object loader
 */
#ifndef _LOADER_H
#define _LOADER_H

#ifdef __GNUG__
#pragma interface
#endif

class Loader {
protected:
        typedef void *(*initfn)();
	initfn load (const char *);
};
#endif
