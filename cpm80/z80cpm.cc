#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <curses.h>

#include "memory.h"
#include "ports.h"
#include "cpu.h"
#include "z80.h"
#include "ram.h"

int dsk[4];
byte seldsk, settrk, setsec, trk = 0xff, sec = 0xff;
word setdma;
WINDOW *mainwin;

class Ports: public PortDevice<z80> {
public:
	Ports(Memory &mem): _mem(mem) {}

	void out(word portw, byte a, z80 *cpu) {
		byte port = portw & 0xff;
		if (port == 4)
			write(1, &a, 1);
		else if (port == 20) {
			seldsk = a;
			trk = sec = 0xff;
		} else if (port == 21)
			settrk = a;
		else if (port == 22)
			setsec = a;
		else if (port == 23)
			setdma = cpu->hl();
		else {
			fprintf(stderr, "out: %d %x\n", port, a);
			fflush(stderr);
		}
	}

	byte in(word portw, z80 *cpu) { 
		byte port = portw & 0xff;
		byte c = 0;
		if (port == 4) {
			read(0, &c, 1);
			if (c == 0x03) {
				delwin(mainwin);
				endwin();
				exit(0);
			}
		} else if (port == 14) {
			// read (from dsk)
			int d = dsk[seldsk];
			if (trk != settrk || sec != setsec) {
//fprintf(stderr, "settrk=%d setsec=%d\n", settrk, setsec);
				trk = settrk;
				sec = setsec;
				lseek(d, 128*(26*trk + sec -1), SEEK_SET);
			}
			byte buf[128];
			int n = read(d, buf, 128);
			sec++;
			c = (n < 0);
			for (int i = 0; i < n; i++)
				_mem[setdma + i] = buf[i];
		} else if (port == 2) {
			// const
			struct timeval tm;
			tm.tv_sec = tm.tv_usec = 0;
			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(0, &fd);
			int n = select(1, &fd, 0, 0, &tm);
			if (0 > n)
				perror("select");
			else if (n == 1)
				c = 0xff;
		}
		else {
			fprintf(stderr, "in: %d\n", port);
			fflush(stderr);
		}
		return c;
	}

private:
	Memory &_mem;
};

int load_com(Memory &memory, const char *file, unsigned short a) {
	int fd = open(file, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return -1;
	}
	unsigned char c;
	while (read(fd, &c, 1) == 1)
		memory[a++] = c;
	close(fd);
	return 0;
}

void status(const char *fmt, ...) {
	char tmp[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(tmp, sizeof(tmp), fmt, args);
	fprintf(stderr, tmp);
	va_end(args);
}

int usage(char *argv[]) {
	fprintf(stderr, "Usage: %s [-d] A.dsk [B.dsk C.dsk D.dsk]\n", argv[0]);
	return -1;
}

int main(int argc, char *argv[])
{
	bool dbg = false;
	int opt;

	while ((opt = getopt(argc, argv, "d")) != -1)
		switch (opt) {
		case 'd':
			dbg = true;
			break;
		default:
			return usage(argv);
		}
	if (optind >= argc)
		return usage(argv);

	mainwin = initscr();
	if (!mainwin) {
		perror("ncurses");
		return -1;
	}

	cbreak();
	raw();
	noecho();

	Memory memory;
	ram ram(65536);
	memory.put(ram, 0x0000);

	if (0 > load_com(memory, "CPM22.bin", 0xe400)) {
		perror("CPM22.bin");
		return -1;
	}

	if (0 > load_com(memory, "CBIOS.bin", 0xfa00)) {
		perror("CBIOS.bin");
		return -1;
	}

	for (int i = 0; optind < argc; i++, optind++) {
		dsk[i] = open(argv[optind], O_RDONLY);
		if (0 > dsk[i]) {
			fprintf(stderr, "open: %s: %s\n", argv[optind], strerror(errno));
			return -1;
		}
	}

	// set up jump to bios
	memory[0] = 0xc3;
	memory[1] = 0x00;
	memory[2] = 0xfa;

	Ports ports(memory);
	z80 cpu(memory, ports);
	cpu.reset();

	while (true)
		cpu.run(1);
}
