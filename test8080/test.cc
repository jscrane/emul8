#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include "memory.h"
#include "ports.h"
#include "cpu.h"
#include "i8080.h"
#include "ram.h"

// FIXME: need to find some nice way of doing this!
// the test program calls BDOS CONOUT via the BDOS entry point at 05H
// http://www.mccm.hetlab.tk/millennium/milc/disk/topic_18.htm
// The function number is in register C: 2 for char output or 9 for string

class Ports: public PortDevice {
public:
	Ports(Memory &mem): _mem(mem) {}

	void out(byte port, byte a, i8080 *cpu) {
		if (port == 0) {
			if (a == 2)
				putchar(cpu->e());
			else if (a == 9) {
				char c;
				word a = cpu->de();
				while ((c = _mem[a++]) != '$')
					putchar(c);
				putchar('\n');
			}
		}
	}
	byte in(byte port, i8080 *) { return 0; }
private:
	Memory &_mem;
};

int load(Memory &memory, const char *file) {
	FILE *f = fopen(file, "r");
	if (!f) {
		perror("fopen");
		return -1;
	}
	char line[520];
	int err = 0;
	while (fgets(line, sizeof(line), f)) {
		char *p = line;
		unsigned n, t, a, a0, a1, x;
		if (*p++ != ':')
			break;
		sscanf(p, "%02x%02x%02x%02x", &n, &a1, &a0, &t);
		if (n == 0 || t == 1)
			break;
		a = a1 << 8 | a0;
		p += 8;
		int cs = n + t + a0 + a1;
		for (int i = 0; i < n; i++) {
			sscanf(p, "%02x", &x);
			p += 2;
			cs += x;
			memory[a++] = x;
		}
		sscanf(p, "%02x", &x);
		cs = (1 + ~(cs & 0xff)) & 0xff;
		if (cs != x) {
			printf("checksum error: %02x %02x\n", cs, x);
			err = -1;
			break;
		}
	}
	fclose(f);
	return err;
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
	if (0 > load(memory, argv[1]))
		return -1;

	jmp_buf ex;
	Ports ports(memory);
	i8080 cpu(memory, &ex, status, ports);
	cpu.reset();
	cpu.run(256);
//	cpu.debug();
	memory[0] = 0x76;	// hlt
	memory[5] = 0x79;	// movac
	memory[6] = 0xd3;	// out
	memory[7] = 0x00;	// port 0
	memory[8] = 0xd9;	// ret
	Memory::address opc = cpu.pc();

	if (!setjmp(ex))
		while (true) {
			cpu.run(1111);
			Memory::address pc = cpu.pc();
			if (pc == opc)
				break;
			opc = pc;
		}

	printf(cpu.status());
}
