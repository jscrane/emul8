#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "memory.h"
#include "cpu.h"
#include "ports.h"
#include "z80.h"
#include "ram.h"

class Ports: public PortDevice<z80> {
public:
	Ports(Memory &mem): _mem(mem) {}

	// not sure what this is all about, spectrum-specific stuff?
	void pre(word p, z80 *c) {
		if ((p & 0xc000) == 0x4000)
			printf("%5d PC %04x\n", c->ts(), p);
		c->ts(1);
	}

	void post(word p, z80 *c) {
		if (p & 0x0001) {
			if ((p & 0xc000) == 0x4000) {
				printf("%5d PC %04x\n", c->ts(), p); c->ts(1);
				printf("%5d PC %04x\n", c->ts(), p); c->ts(1);
				printf("%5d PC %04x\n", c->ts(), p); c->ts(1);
			} else
				c->ts(3);
		} else {
			printf("%5d PC %04x\n", c->ts(), p);
			c->ts(3);
		}
	}
	
	void out(word port, byte a, z80 *cpu) {
		pre(port, cpu);
		printf("%5d PW %04x %02x\n", cpu->ts(), port, a);
		post(port, cpu);
	}

	byte in(word port, z80 *cpu) {
		byte r = port >> 8;
		pre(port, cpu);
		printf("%5d PR %04x %02x\n", cpu->ts(), port, r);
		post(port, cpu);
		return r;
	}

private:
	Memory &_mem;
};

int read_test(FILE *f, z80 &z, Memory &m) {
	unsigned af, bc, de, hl, af_, bc_, de_, hl_, ix, iy, sp, pc, memptr;
	unsigned i, r, iff1, iff2, im;
	unsigned halted, end_tstates2, address;
	char test_name[80];

	do {
		if (!fgets(test_name, sizeof(test_name), f)) {
			if (!feof(f))
				fprintf(stderr, "reading test description: %s\n", strerror(errno));
      			return -1;
		}
	} while (test_name[0] == '\n');

	printf("%s", test_name);

	if (fscanf(f, "%x %x %x %x %x %x %x %x %x %x %x %x %x", 
		&af, &bc, &de, &hl, &af_, &bc_, &de_, &hl_, &ix, &iy, 
		&sp, &pc, &memptr) != 13) 
	{
		fprintf(stderr, "first registers' line corrupt\n");
		return -1;
	}

	z.af(af);	z.bc(bc);	z.de(de);	z.hl(hl);
	z.af_(af_);	z.bc_(bc_);	z.de_(de_);	z.hl_(hl_);
	z.ix(ix);	z.iy(iy);	z.sp(sp);	z.pc(pc);
	z.memptr(memptr);

	if (fscanf(f, "%x %x %u %u %u %d %d", &i, &r, &iff1, &iff2, &im,
		&halted, &end_tstates2 ) != 7) 
	{
		fprintf(stderr, "second registers' line corrupt\n");
		return -1;
	}
	z.i(i);		z.r(r);
	z.iff1(iff1);	z.iff2(iff2);
	z.im(im);

	for (;;) {
		if (fscanf(f, "%x", &address) != 1) {
			fprintf(stderr, "no address found\n");
			return -1;
		}

		if (address >= 0x10000) break;

		for (;;) {
			unsigned byte;

			if (fscanf(f, "%x", &byte) != 1) {
				fprintf(stderr, "no data byte found\n");
				return -1;
			}
    
			if (byte >= 0x100) break;

			m[address++] = byte;
		}
	}
	return end_tstates2;
}

void dump_cpu_state(z80 &z) {
	printf("%04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
		z.af(), z.bc(), z.de(), z.hl(), z.af_(), z.bc_(), z.de_(), 
		z.hl_(), z.ix(), z.iy(), z.sp(), z.pc(), z.memptr());
	printf("%02x %02x %d %d %d %d %d\n",
		z.i(), z.r(), z.iff1(), z.iff2(), z.im(), z.halted(), z.ts());
}

void dump_memory_state(byte b[], Memory &m) {
	for (unsigned i = 0; i < 0x10000; i++) {
		if (m[i] == b[i])
			continue;

		printf("%04x ", i);
		for (; i < 0x10000 && m[i] != b[i]; i++)
			printf("%02x ", (byte)m[i]);
		printf("-1\n");
	}
	printf("\n");
}

int main(int argc, char *argv[]) {

	Memory memory;
	ram ram(65536);
	memory.put(ram, 0x0000);

	Ports ports(memory);
	z80 cpu(memory, ports);
	cpu.reset();

	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		perror("fopen");
		return -1;
	}

	while (true) {
		byte backup[0x10000];
		for (unsigned i = 0; i < 0x10000; ) {
			memory[i++] = 0xde; memory[i++] = 0xad;
			memory[i++] = 0xbe; memory[i++] = 0xef;
		}

		int end_ts = read_test(fp, cpu, memory);
		if (0 > end_ts)
			break;

		for (unsigned i = 0; i < 0x10000; i++)
			backup[i] = memory[i];

		while (cpu.ts() < end_ts)
			cpu.run(1);

		dump_cpu_state(cpu);
		dump_memory_state(backup, memory);
		cpu.reset();
	}

	fclose(fp);
}
