/*
 * memory.h
 */
#ifndef _MEMORY_H
#define _MEMORY_H

#ifdef __GNUG__
#pragma interface
#endif

#ifndef _CHAINABLE_H
#include <chainable.h>
#endif

#ifndef _CHAIN_H
#include <chain.h>
#endif

#ifndef _LOADER_H
#include <loader.h>
#endif

typedef unsigned char byte;
typedef unsigned short word;

class Memory {
public:
	typedef unsigned short address;
	static const int page_size = 256;

	class Device {
	public:
		Device (int pages): _pages(pages) {}
		virtual ~Device () {}
		int pages () const { return _pages; }
		void access (address a) { _acc=a-_base; }
		void base (address a) { _base=a; }
		address base () const { return _base; }

		virtual void operator= (byte) =0;
		virtual operator byte () =0;

		virtual const char *name () const =0;

		class Builder: public Chainable {
		public:
			virtual bool recognises (const char *) =0;
			virtual bool build (Memory *, int, const char **) =0;
		};

		class Loader: public Builder, private ::Loader {
		public:
			Loader (Chain<Builder> &bldrs): _bldrs(bldrs) {}
			bool recognises (const char *);
			bool build (Memory *, int, const char **);
		private:
			Builder *_new;
			Chain<Builder> &_bldrs;
		};
	protected:
		address _acc, _base;
	private:
		int _pages;
	};

	// builders for all currently-known device types
	//
	Chain<Device::Builder> devices;

	// insert a new device instance
	//
	void put (Device &d, address at);
	Device *get (address at) const { return _pages[at/page_size]; }

	// enumerate all device instances
	//
	class Iter {
	public:
		Iter (const Memory *m): _mem(m), _curr(m->get (0)) {}

		operator bool () const { return _curr!=0; }
		operator Device* () { return _curr; }
		const Iter &operator= (const Iter &i) 
			{ _curr=i._curr; return i; }

		Iter operator++ (int);
		Iter &operator-- ();

	private:
		const Memory *_mem;
		Device *_curr;
	};

	// primary access interface
	//
	Device &operator[] (address a) { 
		Device *d = get (a);
		d->access (a);
		return *d;
	}

	Memory (int pages=256);
private:
	Device **_pages;
};

// this class could be used for debugging...
//
class Null: public Memory::Device {
public:
	void operator= (byte) {}
	operator byte () { return 0; }
	Null (int pages = 1): Memory::Device(pages) {}

	const char *name () const { return "empty"; }
};
#endif
