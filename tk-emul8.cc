/*
 * tk-emul8.cc -- extensions to interpreter for basic emulator harness
 */
#include <tcl.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "memory.h"
#include "cpu.h"

/*
 * file globals
 */
static Tcl_Interp *interp;
static bool halt=false;

extern "C" Tcl_Interp *getInterp() { return interp; }

/*
 * commands added to the interpreter
 */
extern "C" int StartCPU(ClientData d, Tcl_Interp *i, int ac, const char **av) {
	CPU *cpu = (CPU *)d;
	for (halt=false; !halt; ) {
		cpu->run (1000);
		Tcl_DoOneEvent (TCL_ALL_EVENTS | TCL_DONT_WAIT);
	}
	return TCL_OK;
}

extern "C" int StopCPU(ClientData, Tcl_Interp *, int, const char **) {
	halt = true;
	return TCL_OK;
}
 
extern "C" int CPU_Status(ClientData d, Tcl_Interp *i, int, const char **) {
	CPU *cpu = (CPU *)d;
	char buf[256];
	Tcl_SetResult (i, cpu->status(buf, sizeof(buf), true), TCL_STATIC);
	return TCL_OK;
}

extern "C" int ResetCPU(ClientData d, Tcl_Interp *, int, const char **) {
	CPU *cpu = (CPU *)d;
	cpu->reset ();
	return TCL_OK;
}

extern "C" void DeleteProcessor(ClientData d) {
	CPU *cpu = (CPU *)d;
	delete cpu;
}

extern "C" int Processor(ClientData d, Tcl_Interp *i, int ac, const char **av) {
	if (ac != 2) {
		Tcl_SetResult (i, "Wrong # args", TCL_STATIC);
		return TCL_ERROR;
	}
	Memory *m = (Memory *)d;
	CPU *cpu = CPU::build (av[1], *m);
	if (!cpu) {
		Tcl_SetResult (i, "Unknown processor: ", TCL_STATIC);
		Tcl_AppendElement (i, av[1]);
		return TCL_ERROR;
	}
	Tcl_CreateCommand (i, "start_cpu", StartCPU, cpu, 0);
	Tcl_CreateCommand (i, "stop_cpu", StopCPU, cpu, 0);
	Tcl_CreateCommand (i, "cpu_state", CPU_Status, cpu, 0);
	Tcl_CreateCommand (i, "reset_cpu", ResetCPU, cpu, DeleteProcessor);
	return TCL_OK;
}

extern "C" int Device(ClientData d, Tcl_Interp *i, int ac, const char **av) {
	Memory *m = (Memory *)d;
	Chain<Memory::Device::Builder>::Iter b = m->devices.begin ();
	for (; b; b++)
		if (b->recognises (av[1])) {
			if (b->build (m, ac-2, av+2))
				return TCL_OK;
			Tcl_SetResult (i, "building ", TCL_STATIC);
			Tcl_AppendElement (i, av[1]);
			return TCL_ERROR;
		}
	Tcl_SetResult (i, "Unknown device type: ", TCL_STATIC);
	Tcl_AppendElement (i, av[1]);
	return TCL_ERROR;
}

extern "C" int Iterator(ClientData d, Tcl_Interp *i, int ac, const char **av) {
	Memory::Iter &m = *(Memory::Iter *)d;
	char buf[80];
	if (ac == 1) {
		sprintf (buf, "%d", (bool)m);
		Tcl_SetResult (i, buf, TCL_VOLATILE);
	} else if (ac != 2) {
		Tcl_SetResult (i, "Wrong # args", TCL_STATIC);
		return TCL_ERROR;
	} else if (!strcmp (av[1], "next")) {
		Memory::Device *d = m++;
		sprintf (buf, "%04x", d->base ());
		Tcl_AppendElement (i, buf);
		sprintf (buf, "%d", d->pages ()*Memory::page_size);
		Tcl_AppendElement (i, buf);
		Tcl_AppendElement (i, (char *)d->name ());
	} else if (!strcmp (av[1], "delete"))
		Tcl_DeleteCommand (i, av[0]);
	else {
		Tcl_SetResult (i, "Unknown operation: ", TCL_STATIC);
		Tcl_AppendElement (i, av[1]);
		return TCL_ERROR;
	}
	return TCL_OK;
}

extern "C" void DeleteIterator(ClientData d) {
	Memory::Iter *m = (Memory::Iter *)d;
	delete m;
}

extern "C" int Query(ClientData d, Tcl_Interp *i, int ac, const char **av) {
	if (ac != 2) {
		Tcl_SetResult (i, "Wrong # args", TCL_STATIC);
		return TCL_ERROR;
	}
	Memory::Iter *itp = new Memory::Iter((Memory *)d);
	Tcl_CreateCommand (i, av[1], Iterator, itp, DeleteIterator);
	Tcl_AppendElement (i, av[1]);
	return TCL_OK;
}

/*
 * called when the script invokes "load tk-emul8.so Emulator"
 */
extern "C" int Emulator_Init(Tcl_Interp *i) {
	Memory *memory = new Memory;
	Tcl_CreateCommand (i, "device", Device, memory, 0);
	Tcl_CreateCommand (i, "query", Query, memory, 0);
	Tcl_CreateCommand (i, "processor", Processor, memory, 0);
	interp = i;
	return TCL_OK;
}
