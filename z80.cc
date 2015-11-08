#include <stdio.h>

#include "memory.h"
#include "ports.h"
#include "cpu.h"
#include "z80.h"

#define CPU_STATE_FMT "%04x %02x %02x %04x %04x %04x %04x %d%d%d%d%d%d%d%d\r\n",\
			PC, op, A, BC, DE, HL, SP, flags.S, flags.Z,\
			flags._5, flags.H, flags._3, flags.P, flags.N, flags.C

void z80::_step(OP ops[]) {
	_mc(PC, 4);
	byte op = _mem[PC];
#if defined(CPU_DEBUG)
	printf("%5d MR %04x %02x\n", _ts, PC, op);
#endif
	PC++;
	R++;
	(this->*ops[op])();
}

void z80::run(unsigned clocks) {
	while (clocks--)
		step();
}

void z80::reset() {
	AF = BC = DE = HL = PC = SP = 0;
	AF_ = BC_ = DE_ = HL_ = IX = IY = 0;
	I = R = 0;
	_irq_pending = 0;
	_ts = 0;
	_halted = false;
}

void z80::raise(int level) {
/* FIXME: interrupts
	if (flags.I) {
		flags.I = 0;
		_irq_pending = 0;
		_push(PC);
		PC = level * 8;
	} else
		_irq_pending = level;
*/
}

void z80::ei() {
/* FIXME: interrupts
	flags.I = 1;
	if (_irq_pending)
		raise(_irq_pending);
*/
}

char *z80::status() {
	static char buf[128];
	byte op = _mem[PC];
	sprintf(buf, "_pc_ op aa _bc_ _de_ _hl_ _sp_ szih_p_c\r\n" CPU_STATE_FMT);
	return buf;
}

void z80::daa() {
	byte c = flags.C, a = 0, hi = (A & 0xf0) >> 4, lo = A & 0x0f;
	if (flags.H || lo > 9)
		a = 0x06;
	if (c || A > 0x99) {
		a |= 0x60;
		c = 1;
	}
	if (flags.N)
		_sub(a);
	else
		_add(a);
	flags.C = c;
	flags.P = parity_table[A];
}

/* FIXME?
void z80::halt() {
	_status("CPU halted at %04x\r\n%s", (PC-1), status());
	longjmp(*_err, 1);
}
*/

void z80::_step_idx(OP_IDX ops[]) {

	_mc(PC, 3);
	byte off = _mem[PC];
#if defined(CPU_DEBUG)
	printf("%5d MR %04x %02x\n", _ts, PC, off);
#endif
	PC++;

	_mc(PC, 3);
	byte op = _mem[PC];
#if defined(CPU_DEBUG)
	printf("%5d MR %04x %02x\n", _ts, PC, op);
#endif
	_mc(PC, 1);
	_mc(PC, 1);
	PC++;
	(this->*ops[op])(off);
}

void z80::dd() {
	_mc(PC, 4);
	byte op = _mem[PC];
#if defined(CPU_DEBUG)
	printf("%5d MR %04x %02x\n", _ts, PC, op);
#endif
	PC++;
	R++;
	switch (op) {
	case 0xcb:
		_step_idx(_ddcb);
		break;
	// FIXME
	}
}

void z80::ed() {
	// FIXME
	halt();
}

void z80::fd() {
	// FIXME
	halt();
}

int z80::parity_table[] = {
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
};

z80::z80(Memory &m, jmp_buf *jb, CPU::statfn s, PortDevice<z80> &ports): CPU(m, jb, s)
{
	_ports = &ports;
	_debug = false;

	OP *p = _ops;

	// 0x00
	*p++ = &z80::nop; *p++ = &z80::ldbcpc; 
	*p++ = &z80::ldBCa; *p++ = &z80::incbc;
	*p++ = &z80::incb; *p++ = &z80::decb;
	*p++ = &z80::ldb; *p++ = &z80::rlca;
	*p++ = &z80::exaf; *p++ = &z80::addhlbc;
	*p++ = &z80::ldaBC; *p++ = &z80::decbc;
	*p++ = &z80::incc; *p++ = &z80::decc;
	*p++ = &z80::ldc; *p++ = &z80::rrca;

	// 0x10
	*p++ = &z80::djnz; *p++ = &z80::lddepc; 
	*p++ = &z80::ldDEa; *p++ = &z80::incde;
	*p++ = &z80::incd; *p++ = &z80::decd;
	*p++ = &z80::ldd; *p++ = &z80::rla;
	*p++ = &z80::jr; *p++ = &z80::addhlde;
	*p++ = &z80::ldaDE; *p++ = &z80::decde;
	*p++ = &z80::ince; *p++ = &z80::dece;
	*p++ = &z80::lde; *p++ = &z80::rra;

	// 0x20
	*p++ = &z80::jrnz; *p++ = &z80::ldhlpc;
	*p++ = &z80::ldPChl; *p++ = &z80::inchl;
	*p++ = &z80::inch; *p++ = &z80::dech;
	*p++ = &z80::ldh; *p++ = &z80::daa;
	*p++ = &z80::jrz; *p++ = &z80::addhlhl; 
	*p++ = &z80::ldhlPC; *p++ = &z80::dechl; 
	*p++ = &z80::incl; *p++ = &z80::decl; 
	*p++ = &z80::ldl; *p++ = &z80::cpl; 

	// 0x30
	*p++ = &z80::jrnc; *p++ = &z80::ldsppc;
	*p++ = &z80::ldPCa; *p++ = &z80::incsp;
	*p++ = &z80::incHL; *p++ = &z80::decHL;
	*p++ = &z80::ldHL; *p++ = &z80::scf;
	*p++ = &z80::jrc; *p++ = &z80::addhlsp;
	*p++ = &z80::ldaPC; *p++ = &z80::decsp;
	*p++ = &z80::inca; *p++ = &z80::deca;
	*p++ = &z80::lda; *p++ = &z80::ccf;

	// 0x40
	*p++ = &z80::ldbb; *p++ = &z80::ldbc;
	*p++ = &z80::ldbd; *p++ = &z80::ldbe;
	*p++ = &z80::ldbh; *p++ = &z80::ldbl;
	*p++ = &z80::ldbHL; *p++ = &z80::ldba;
	*p++ = &z80::ldcb; *p++ = &z80::ldcc;
	*p++ = &z80::ldcd; *p++ = &z80::ldce;
	*p++ = &z80::ldch; *p++ = &z80::ldcl;
	*p++ = &z80::ldcHL; *p++ = &z80::ldca;

	// 0x50
	*p++ = &z80::lddb; *p++ = &z80::lddc;
	*p++ = &z80::lddd; *p++ = &z80::ldde;
	*p++ = &z80::lddh; *p++ = &z80::lddl;
	*p++ = &z80::lddHL; *p++ = &z80::ldda;
	*p++ = &z80::ldeb; *p++ = &z80::ldec;
	*p++ = &z80::lded; *p++ = &z80::ldee;
	*p++ = &z80::ldeh; *p++ = &z80::ldel;
	*p++ = &z80::ldeHL; *p++ = &z80::ldea;

	// 0x60
	*p++ = &z80::ldhb; *p++ = &z80::ldhc;
	*p++ = &z80::ldhd; *p++ = &z80::ldhe;
	*p++ = &z80::ldhh; *p++ = &z80::ldhl;
	*p++ = &z80::ldhHL; *p++ = &z80::ldha;
	*p++ = &z80::ldlb; *p++ = &z80::ldlc;
	*p++ = &z80::ldld; *p++ = &z80::ldle;
	*p++ = &z80::ldlh; *p++ = &z80::ldll;
	*p++ = &z80::ldlHL; *p++ = &z80::ldla;

	// 0x70
	*p++ = &z80::ldHLb; *p++ = &z80::ldHLc;
	*p++ = &z80::ldHLd; *p++ = &z80::ldHLe;
	*p++ = &z80::ldHLh; *p++ = &z80::ldHLl;
	*p++ = &z80::halt; *p++ = &z80::ldHLa;
	*p++ = &z80::ldab; *p++ = &z80::ldac;
	*p++ = &z80::ldad; *p++ = &z80::ldae;
	*p++ = &z80::ldah; *p++ = &z80::ldal;
	*p++ = &z80::ldaHL; *p++ = &z80::ldaa;

	// 0x80
	*p++ = &z80::addab; *p++ = &z80::addac;
	*p++ = &z80::addad; *p++ = &z80::addae;
	*p++ = &z80::addah; *p++ = &z80::addal;
	*p++ = &z80::addaHL; *p++ = &z80::addaa;
	*p++ = &z80::adcab; *p++ = &z80::adcac;
	*p++ = &z80::adcad; *p++ = &z80::adcae;
	*p++ = &z80::adcah; *p++ = &z80::adcal;
	*p++ = &z80::adcaHL; *p++ = &z80::adcaa;

	// 0x90
	*p++ = &z80::subab; *p++ = &z80::subac;
	*p++ = &z80::subad; *p++ = &z80::subae;
	*p++ = &z80::subah; *p++ = &z80::subal;
	*p++ = &z80::subaHL; *p++ = &z80::subaa;
	*p++ = &z80::sbcab; *p++ = &z80::sbcac;
	*p++ = &z80::sbcad; *p++ = &z80::sbcae;
	*p++ = &z80::sbcah; *p++ = &z80::sbcal;
	*p++ = &z80::sbcaHL; *p++ = &z80::sbcaa;

	// 0xa0
	*p++ = &z80::andb; *p++ = &z80::andc;
	*p++ = &z80::andd; *p++ = &z80::ande;
	*p++ = &z80::andh; *p++ = &z80::andl;
	*p++ = &z80::andHL; *p++ = &z80::anda;
	*p++ = &z80::xorb; *p++ = &z80::xorc;
	*p++ = &z80::xord; *p++ = &z80::xore;
	*p++ = &z80::xorh; *p++ = &z80::xorl;
	*p++ = &z80::xorHL; *p++ = &z80::xora;

	// 0xb0
	*p++ = &z80::orb; *p++ = &z80::orc;
	*p++ = &z80::ord; *p++ = &z80::ore;
	*p++ = &z80::orh; *p++ = &z80::orl;
	*p++ = &z80::orHL; *p++ = &z80::ora;
	*p++ = &z80::cpb; *p++ = &z80::cpc;
	*p++ = &z80::cpd; *p++ = &z80::cpe;
	*p++ = &z80::cph; *p++ = &z80::cpL;
	*p++ = &z80::cpHL; *p++ = &z80::cpa;

	// 0xc0
	*p++ = &z80::retnz; *p++ = &z80::popbc;
	*p++ = &z80::jpnz; *p++ = &z80::jp;
	*p++ = &z80::callnz; *p++ = &z80::pushbc;
	*p++ = &z80::adda; *p++ = &z80::rst00;
	*p++ = &z80::retz; *p++ = &z80::ret;
	*p++ = &z80::jpz; *p++ = &z80::cb;
	*p++ = &z80::callz; *p++ = &z80::call;
	*p++ = &z80::adca; *p++ = &z80::rst08;

	// 0xd0
	*p++ = &z80::retnc; *p++ = &z80::popde;
	*p++ = &z80::jpnc; *p++ = &z80::outa;
	*p++ = &z80::callnc; *p++ = &z80::pushde;
	*p++ = &z80::suba; *p++ = &z80::rst10;
	*p++ = &z80::retc; *p++ = &z80::exx;
	*p++ = &z80::jpc; *p++ = &z80::ina;
	*p++ = &z80::callc; *p++ = &z80::dd;
	*p++ = &z80::sbca; *p++ = &z80::rst18;

	// 0xe0
	*p++ = &z80::retpo; *p++ = &z80::pophl;
	*p++ = &z80::jppo; *p++ = &z80::exSPhl;
	*p++ = &z80::callpo; *p++ = &z80::pushhl;
	*p++ = &z80::and; *p++ = &z80::rst20;
	*p++ = &z80::retpe; *p++ = &z80::jphl;
	*p++ = &z80::jppe; *p++ = &z80::exdehl;
	*p++ = &z80::callpe; *p++ = &z80::ed;
	*p++ = &z80::xor; *p++ = &z80::rst28;

	// 0xf0
	*p++ = &z80::retp; *p++ = &z80::popaf;
	*p++ = &z80::jpp; *p++ = &z80::di;
	*p++ = &z80::callp; *p++ = &z80::pushaf;
	*p++ = &z80::or; *p++ = &z80::rst30;
	*p++ = &z80::retm; *p++ = &z80::ldsphl;
	*p++ = &z80::jpm; *p++ = &z80::ei;
	*p++ = &z80::callm; *p++ = &z80::fd;
	*p++ = &z80::cp; *p++ = &z80::rst38;

	OP *c = _cb;

	// 0x00
	*c++ = &z80::rlcB; *c++ = &z80::rlcC;
	*c++ = &z80::rlcD; *c++ = &z80::rlcE;
	*c++ = &z80::rlcH; *c++ = &z80::rlcL;
	*c++ = &z80::rlcHL; *c++ = &z80::rlcA;
	*c++ = &z80::rrcB; *c++ = &z80::rrcC;
	*c++ = &z80::rrcD; *c++ = &z80::rrcE;
	*c++ = &z80::rrcH; *c++ = &z80::rrcL;
	*c++ = &z80::rrcHL; *c++ = &z80::rrcA;

	// 0x10
	*c++ = &z80::rlB; *c++ = &z80::rlC;
	*c++ = &z80::rlD; *c++ = &z80::rlE;
	*c++ = &z80::rlH; *c++ = &z80::rlL;
	*c++ = &z80::rlHL; *c++ = &z80::rlA;
	*c++ = &z80::rrB; *c++ = &z80::rrC;
	*c++ = &z80::rrD; *c++ = &z80::rrE;
	*c++ = &z80::rrH; *c++ = &z80::rrL;
	*c++ = &z80::rrHL; *c++ = &z80::rrA;

	// 0x20
	*c++ = &z80::slab; *c++ = &z80::slac;
	*c++ = &z80::slad; *c++ = &z80::slae;
	*c++ = &z80::slah; *c++ = &z80::slal;
	*c++ = &z80::slaHL; *c++ = &z80::slaa;
	*c++ = &z80::srab; *c++ = &z80::srac;
	*c++ = &z80::srad; *c++ = &z80::srae;
	*c++ = &z80::srah; *c++ = &z80::sral;
	*c++ = &z80::sraHL; *c++ = &z80::sraa;

	// 0x30
	*c++ = &z80::sllb; *c++ = &z80::sllc;
	*c++ = &z80::slld; *c++ = &z80::slle;
	*c++ = &z80::sllh; *c++ = &z80::slll;
	*c++ = &z80::sllHL; *c++ = &z80::slla;
	*c++ = &z80::srlb; *c++ = &z80::srlc;
	*c++ = &z80::srld; *c++ = &z80::srle;
	*c++ = &z80::srlh; *c++ = &z80::srll;
	*c++ = &z80::srlHL; *c++ = &z80::srla;

	// 0x40
	*c++ = &z80::bit0b; *c++ = &z80::bit0c;
	*c++ = &z80::bit0d; *c++ = &z80::bit0e;
	*c++ = &z80::bit0h; *c++ = &z80::bit0l;
	*c++ = &z80::bit0HL; *c++ = &z80::bit0a;
	*c++ = &z80::bit1b; *c++ = &z80::bit1c;
	*c++ = &z80::bit1d; *c++ = &z80::bit1e;
	*c++ = &z80::bit1h; *c++ = &z80::bit1l;
	*c++ = &z80::bit1HL; *c++ = &z80::bit1a;

	// 0x50
	*c++ = &z80::bit2b; *c++ = &z80::bit2c;
	*c++ = &z80::bit2d; *c++ = &z80::bit2e;
	*c++ = &z80::bit2h; *c++ = &z80::bit2l;
	*c++ = &z80::bit2HL; *c++ = &z80::bit2a;
	*c++ = &z80::bit3b; *c++ = &z80::bit3c;
	*c++ = &z80::bit3d; *c++ = &z80::bit3e;
	*c++ = &z80::bit3h; *c++ = &z80::bit3l;
	*c++ = &z80::bit3HL; *c++ = &z80::bit3a;

	// 0x60
	*c++ = &z80::bit4b; *c++ = &z80::bit4c;
	*c++ = &z80::bit4d; *c++ = &z80::bit4e;
	*c++ = &z80::bit4h; *c++ = &z80::bit4l;
	*c++ = &z80::bit4HL; *c++ = &z80::bit4a;
	*c++ = &z80::bit5b; *c++ = &z80::bit5c;
	*c++ = &z80::bit5d; *c++ = &z80::bit5e;
	*c++ = &z80::bit5h; *c++ = &z80::bit5l;
	*c++ = &z80::bit5HL; *c++ = &z80::bit5a;

	// 0x70
	*c++ = &z80::bit6b; *c++ = &z80::bit6c;
	*c++ = &z80::bit6d; *c++ = &z80::bit6e;
	*c++ = &z80::bit6h; *c++ = &z80::bit6l;
	*c++ = &z80::bit6HL; *c++ = &z80::bit6a;
	*c++ = &z80::bit7b; *c++ = &z80::bit7c;
	*c++ = &z80::bit7d; *c++ = &z80::bit7e;
	*c++ = &z80::bit7h; *c++ = &z80::bit7l;
	*c++ = &z80::bit7HL; *c++ = &z80::bit7a;

	// 0x80
	*c++ = &z80::res0b; *c++ = &z80::res0c;
	*c++ = &z80::res0d; *c++ = &z80::res0e;
	*c++ = &z80::res0h; *c++ = &z80::res0l;
	*c++ = &z80::res0HL; *c++ = &z80::res0a;
	*c++ = &z80::res1b; *c++ = &z80::res1c;
	*c++ = &z80::res1d; *c++ = &z80::res1e;
	*c++ = &z80::res1h; *c++ = &z80::res1l;
	*c++ = &z80::res1HL; *c++ = &z80::res1a;

	// 0x90
	*c++ = &z80::res2b; *c++ = &z80::res2c;
	*c++ = &z80::res2d; *c++ = &z80::res2e;
	*c++ = &z80::res2h; *c++ = &z80::res2l;
	*c++ = &z80::res2HL; *c++ = &z80::res2a;
	*c++ = &z80::res3b; *c++ = &z80::res3c;
	*c++ = &z80::res3d; *c++ = &z80::res3e;
	*c++ = &z80::res3h; *c++ = &z80::res3l;
	*c++ = &z80::res3HL; *c++ = &z80::res3a;

	// 0xa0
	*c++ = &z80::res4b; *c++ = &z80::res4c;
	*c++ = &z80::res4d; *c++ = &z80::res4e;
	*c++ = &z80::res4h; *c++ = &z80::res4l;
	*c++ = &z80::res4HL; *c++ = &z80::res4a;
	*c++ = &z80::res5b; *c++ = &z80::res5c;
	*c++ = &z80::res5d; *c++ = &z80::res5e;
	*c++ = &z80::res5h; *c++ = &z80::res5l;
	*c++ = &z80::res5HL; *c++ = &z80::res5a;

	// 0xb0
	*c++ = &z80::res6b; *c++ = &z80::res6c;
	*c++ = &z80::res6d; *c++ = &z80::res6e;
	*c++ = &z80::res6h; *c++ = &z80::res6l;
	*c++ = &z80::res6HL; *c++ = &z80::res6a;
	*c++ = &z80::res7b; *c++ = &z80::res7c;
	*c++ = &z80::res7d; *c++ = &z80::res7e;
	*c++ = &z80::res7h; *c++ = &z80::res7l;
	*c++ = &z80::res7HL; *c++ = &z80::res7a;

	// 0xc0
	*c++ = &z80::set0b; *c++ = &z80::set0c;
	*c++ = &z80::set0d; *c++ = &z80::set0e;
	*c++ = &z80::set0h; *c++ = &z80::set0l;
	*c++ = &z80::set0HL; *c++ = &z80::set0a;
	*c++ = &z80::set1b; *c++ = &z80::set1c;
	*c++ = &z80::set1d; *c++ = &z80::set1e;
	*c++ = &z80::set1h; *c++ = &z80::set1l;
	*c++ = &z80::set1HL; *c++ = &z80::set1a;

	// 0xd0
	*c++ = &z80::set2b; *c++ = &z80::set2c;
	*c++ = &z80::set2d; *c++ = &z80::set2e;
	*c++ = &z80::set2h; *c++ = &z80::set2l;
	*c++ = &z80::set2HL; *c++ = &z80::set2a;
	*c++ = &z80::set3b; *c++ = &z80::set3c;
	*c++ = &z80::set3d; *c++ = &z80::set3e;
	*c++ = &z80::set3h; *c++ = &z80::set3l;
	*c++ = &z80::set3HL; *c++ = &z80::set3a;

	// 0xe0
	*c++ = &z80::set4b; *c++ = &z80::set4c;
	*c++ = &z80::set4d; *c++ = &z80::set4e;
	*c++ = &z80::set4h; *c++ = &z80::set4l;
	*c++ = &z80::set4HL; *c++ = &z80::set4a;
	*c++ = &z80::set5b; *c++ = &z80::set5c;
	*c++ = &z80::set5d; *c++ = &z80::set5e;
	*c++ = &z80::set5h; *c++ = &z80::set5l;
	*c++ = &z80::set5HL; *c++ = &z80::set5a;

	// 0xf0
	*c++ = &z80::set6b; *c++ = &z80::set6c;
	*c++ = &z80::set6d; *c++ = &z80::set6e;
	*c++ = &z80::set6h; *c++ = &z80::set6l;
	*c++ = &z80::set6HL; *c++ = &z80::set6a;
	*c++ = &z80::set7b; *c++ = &z80::set7c;
	*c++ = &z80::set7d; *c++ = &z80::set7e;
	*c++ = &z80::set7h; *c++ = &z80::set7l;
	*c++ = &z80::set7HL; *c++ = &z80::set7a;

	OP_IDX *d = _ddcb;

	// 0x00
	*d++ = &z80::rlcIXB; *d++ = &z80::rlcIXC;
	*d++ = &z80::rlcIXD; *d++ = &z80::rlcIXE;
	*d++ = &z80::rlcIXH; *d++ = &z80::rlcIXL;
	*d++ = &z80::rlcIX; *d++ = &z80::rlcIXA;
	*d++ = &z80::rrcIXB; *d++ = &z80::rrcIXC;
	*d++ = &z80::rrcIXD; *d++ = &z80::rrcIXE;
	*d++ = &z80::rrcIXH; *d++ = &z80::rrcIXL;
	*d++ = &z80::rrcIX; *d++ = &z80::rrcIXA;

	// 0x10
	*d++ = &z80::rlIXB; *d++ = &z80::rlIXC;
	*d++ = &z80::rlIXD; *d++ = &z80::rlIXE;
	*d++ = &z80::rlIXH; *d++ = &z80::rlIXL;
	*d++ = &z80::rlIX; *d++ = &z80::rlIXA;
	*d++ = &z80::rrIXB; *d++ = &z80::rrIXC;
	*d++ = &z80::rrIXD; *d++ = &z80::rrIXE;
	*d++ = &z80::rrIXH; *d++ = &z80::rrIXL;
	*d++ = &z80::rrIX; *d++ = &z80::rrIXA;

	// 0x20
	*d++ = &z80::slaIXB; *d++ = &z80::slaIXC;
	*d++ = &z80::slaIXD; *d++ = &z80::slaIXE;
	*d++ = &z80::slaIXH; *d++ = &z80::slaIXL;
	*d++ = &z80::slaIX; *d++ = &z80::slaIXA;
	*d++ = &z80::sraIXB; *d++ = &z80::sraIXC;
	*d++ = &z80::sraIXD; *d++ = &z80::sraIXE;
	*d++ = &z80::sraIXH; *d++ = &z80::sraIXL;
	*d++ = &z80::sraIX; *d++ = &z80::sraIXA;

	// 0x30
	*d++ = &z80::sllIXB; *d++ = &z80::sllIXC;
	*d++ = &z80::sllIXD; *d++ = &z80::sllIXE;
	*d++ = &z80::sllIXH; *d++ = &z80::sllIXL;
	*d++ = &z80::sllIX; *d++ = &z80::sllIXA;
	*d++ = &z80::srlIXB; *d++ = &z80::srlIXC;
	*d++ = &z80::srlIXD; *d++ = &z80::srlIXE;
	*d++ = &z80::srlIXH; *d++ = &z80::srlIXL;
	*d++ = &z80::srlIX; *d++ = &z80::srlIXA;

	// 0x40
	*d++ = &z80::bit0IX; *d++ = &z80::bit0IX;
	*d++ = &z80::bit0IX; *d++ = &z80::bit0IX;
	*d++ = &z80::bit0IX; *d++ = &z80::bit0IX;
	*d++ = &z80::bit0IX; *d++ = &z80::bit0IX;
	*d++ = &z80::bit1IX; *d++ = &z80::bit1IX;
	*d++ = &z80::bit1IX; *d++ = &z80::bit1IX;
	*d++ = &z80::bit1IX; *d++ = &z80::bit1IX;
	*d++ = &z80::bit1IX; *d++ = &z80::bit1IX;

	// 0x50
	*d++ = &z80::bit2IX; *d++ = &z80::bit2IX;
	*d++ = &z80::bit2IX; *d++ = &z80::bit2IX;
	*d++ = &z80::bit2IX; *d++ = &z80::bit2IX;
	*d++ = &z80::bit2IX; *d++ = &z80::bit2IX;
	*d++ = &z80::bit3IX; *d++ = &z80::bit3IX;
	*d++ = &z80::bit3IX; *d++ = &z80::bit3IX;
	*d++ = &z80::bit3IX; *d++ = &z80::bit3IX;
	*d++ = &z80::bit3IX; *d++ = &z80::bit3IX;

	// 0x60
	*d++ = &z80::bit4IX; *d++ = &z80::bit4IX;
	*d++ = &z80::bit4IX; *d++ = &z80::bit4IX;
	*d++ = &z80::bit4IX; *d++ = &z80::bit4IX;
	*d++ = &z80::bit4IX; *d++ = &z80::bit4IX;
	*d++ = &z80::bit5IX; *d++ = &z80::bit5IX;
	*d++ = &z80::bit5IX; *d++ = &z80::bit5IX;
	*d++ = &z80::bit5IX; *d++ = &z80::bit5IX;
	*d++ = &z80::bit5IX; *d++ = &z80::bit5IX;

	// 0x70
	*d++ = &z80::bit6IX; *d++ = &z80::bit6IX;
	*d++ = &z80::bit6IX; *d++ = &z80::bit6IX;
	*d++ = &z80::bit6IX; *d++ = &z80::bit6IX;
	*d++ = &z80::bit6IX; *d++ = &z80::bit6IX;
	*d++ = &z80::bit7IX; *d++ = &z80::bit7IX;
	*d++ = &z80::bit7IX; *d++ = &z80::bit7IX;
	*d++ = &z80::bit7IX; *d++ = &z80::bit7IX;
	*d++ = &z80::bit7IX; *d++ = &z80::bit7IX;

	// 0x80
	*d++ = &z80::res0IXB; *d++ = &z80::res0IXC;
	*d++ = &z80::res0IXD; *d++ = &z80::res0IXE;
	*d++ = &z80::res0IXH; *d++ = &z80::res0IXL;
	*d++ = &z80::res0IX; *d++ = &z80::res0IXA;
	*d++ = &z80::res1IXB; *d++ = &z80::res1IXC;
	*d++ = &z80::res1IXD; *d++ = &z80::res1IXE;
	*d++ = &z80::res1IXH; *d++ = &z80::res1IXL;
	*d++ = &z80::res1IX; *d++ = &z80::res1IXA;

	// 0x90
	*d++ = &z80::res2IXB; *d++ = &z80::res2IXC;
	*d++ = &z80::res2IXD; *d++ = &z80::res2IXE;
	*d++ = &z80::res2IXH; *d++ = &z80::res2IXL;
	*d++ = &z80::res2IX; *d++ = &z80::res2IXA;
	*d++ = &z80::res3IXB; *d++ = &z80::res3IXC;
	*d++ = &z80::res3IXD; *d++ = &z80::res3IXE;
	*d++ = &z80::res3IXH; *d++ = &z80::res3IXL;
	*d++ = &z80::res3IX; *d++ = &z80::res3IXA;

	// 0xa0
	*d++ = &z80::res4IXB; *d++ = &z80::res4IXC;
	*d++ = &z80::res4IXD; *d++ = &z80::res4IXE;
	*d++ = &z80::res4IXH; *d++ = &z80::res4IXL;
	*d++ = &z80::res4IX; *d++ = &z80::res4IXA;
	*d++ = &z80::res5IXB; *d++ = &z80::res5IXC;
	*d++ = &z80::res5IXD; *d++ = &z80::res5IXE;
	*d++ = &z80::res5IXH; *d++ = &z80::res5IXL;
	*d++ = &z80::res5IX; *d++ = &z80::res5IXA;

	// 0xb0
	*d++ = &z80::res6IXB; *d++ = &z80::res6IXC;
	*d++ = &z80::res6IXD; *d++ = &z80::res6IXE;
	*d++ = &z80::res6IXH; *d++ = &z80::res6IXL;
	*d++ = &z80::res6IX; *d++ = &z80::res6IXA;
	*d++ = &z80::res7IXB; *d++ = &z80::res7IXC;
	*d++ = &z80::res7IXD; *d++ = &z80::res7IXE;
	*d++ = &z80::res7IXH; *d++ = &z80::res7IXL;
	*d++ = &z80::res7IX; *d++ = &z80::res7IXA;

	// 0xc0
	*d++ = &z80::set0IXB; *d++ = &z80::set0IXC;
	*d++ = &z80::set0IXD; *d++ = &z80::set0IXE;
	*d++ = &z80::set0IXH; *d++ = &z80::set0IXL;
	*d++ = &z80::set0IX; *d++ = &z80::set0IXA;
	*d++ = &z80::set1IXB; *d++ = &z80::set1IXC;
	*d++ = &z80::set1IXD; *d++ = &z80::set1IXE;
	*d++ = &z80::set1IXH; *d++ = &z80::set1IXL;
	*d++ = &z80::set1IX; *d++ = &z80::set1IXA;

	// 0xd0
	*d++ = &z80::set2IXB; *d++ = &z80::set2IXC;
	*d++ = &z80::set2IXD; *d++ = &z80::set2IXE;
	*d++ = &z80::set2IXH; *d++ = &z80::set2IXL;
	*d++ = &z80::set2IX; *d++ = &z80::set2IXA;
	*d++ = &z80::set3IXB; *d++ = &z80::set3IXC;
	*d++ = &z80::set3IXD; *d++ = &z80::set3IXE;
	*d++ = &z80::set3IXH; *d++ = &z80::set3IXL;
	*d++ = &z80::set3IX; *d++ = &z80::set3IXA;

	// 0xe0
	*d++ = &z80::set4IXB; *d++ = &z80::set4IXC;
	*d++ = &z80::set4IXD; *d++ = &z80::set4IXE;
	*d++ = &z80::set4IXH; *d++ = &z80::set4IXL;
	*d++ = &z80::set4IX; *d++ = &z80::set4IXA;
	*d++ = &z80::set5IXB; *d++ = &z80::set5IXC;
	*d++ = &z80::set5IXD; *d++ = &z80::set5IXE;
	*d++ = &z80::set5IXH; *d++ = &z80::set5IXL;
	*d++ = &z80::set5IX; *d++ = &z80::set5IXA;

	// 0xf0
	*d++ = &z80::set6IXB; *d++ = &z80::set6IXC;
	*d++ = &z80::set6IXD; *d++ = &z80::set6IXE;
	*d++ = &z80::set6IXH; *d++ = &z80::set6IXL;
	*d++ = &z80::set6IX; *d++ = &z80::set6IXA;
	*d++ = &z80::set7IXB; *d++ = &z80::set7IXC;
	*d++ = &z80::set7IXD; *d++ = &z80::set7IXE;
	*d++ = &z80::set7IXH; *d++ = &z80::set7IXL;
	*d++ = &z80::set7IX; *d++ = &z80::set7IXA;
}