Requirements
------------
This emulator was built on RedHat Linux 4.2 (kernel 2.0.30) with the
following installed software:
- gcc version 2.7.2.1
- gnu make 3.75
- Tcl/Tk versions 7.6 and 4.2 respectively

It has also been successfully built on RedHat Linux 5.0 (kernel 2.0.32)
with:
- gcc version 2.7.2.3
- gnu make 3.76.1
- Tcl/Tk versions 8.0

(Note: dialog.tcl is expected to be in /usr/lib/tk.  On my setup I have
made this a soft-link to the real location, viz. /usr/lib/tk8.0)

Environment
-----------
The Makefile is configured by the following environment variables:
- MACHINE: the host cpu type (e.g. i686)
- OS: the host operating system (e.g. Linux)

Running the emulator
--------------------
The LD_LIBRARY_PATH environment variable must contain the shared objects' 
location, e.g., if the emulator is to be executed in situ:

```bash
$ export LD_LIBRARY_PATH=$OS-$MACHINE:$LD_LIBRARY_PATH
$ ./tk-emul8 uk101
```

The emulator takes an optional parameter naming either an initial 
configuration (uk101/mon02.tcl) or a directory containing a file
called config.tcl.
