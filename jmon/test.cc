#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>

#include "memory.h"
#include "ports.h"
#include "cpu.h"
#include "i8080.h"
#include "ram.h"

// port emulation of an altair 8080, see 
// http://jefftranter.blogspot.ie/2014/03/jmon-8080-version.html
class Ports: public PortDevice {
public:
	Ports(Memory &mem): _mem(mem) {}

	void out(byte port, byte a, i8080 *cpu) {
		if (port == 0x11) {
			write(1, &a, 1);
		}
		else
			printf("out: %d %02x\n", port, a);
	}

	byte in(byte port, i8080 *) { 
		if (port == 0x10) {
			byte status = 0x02;	// TDRE
			struct timeval tm;
			tm.tv_sec = tm.tv_usec = 0;
			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(0, &fd);
			if (select(1, &fd, 0, 0, &tm) == 1)
				status |= 0x01;	// RDRF
			return status;
		} 
		if (port == 0x11) {
			byte b = 0;
			read(0, &b, 1);
			return b;
		}
		printf("in: %d\n", port);
		return 0; 
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

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s image\n", argv[0]);
		return -1;
	}
	
	Memory memory;
	ram ram(65536);
	memory.put(ram, 0x0000);
	if (0 > load_com(memory, argv[1], 0x0000))
		return -1;

	jmp_buf ex;
	Ports ports(memory);
	i8080 cpu(memory, &ex, status, ports);
	cpu.reset();

	if (!setjmp(ex))
		while (true)
			cpu.run(1);
}
