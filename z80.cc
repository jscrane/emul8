#include <stdio.h>

#include "memory.h"
#include "ports.h"
#include "cpu.h"
#include "z80.h"

#define CPU_STATE_FMT "%04x %02x %02x %04x %04x %04x %04x %d%d%d%d%d%d%d%d\r\n",\
			PC, op, A, BC, DE, HL, SP, flags.S, flags.Z,\
			flags.I, flags.H, flags._, flags.P, flags.N, flags.C

void z80::step() {
	_mc(PC, 4);
	byte op = _mem[PC];
	printf("%5d MR %04x %02x\n", _ts, PC, op);
#if defined(CPU_DEBUG)
	if (_debug)
		_status(CPU_STATE_FMT);
#endif
	PC++;
	R++;
	(this->*_ops[op])();
}

void z80::run(unsigned clocks) {
#if defined(CPU_DEBUG)
	if (_debug) {
		step();
		return;
	}
#endif
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
	if (flags.I) {
		flags.I = 0;
		_irq_pending = 0;
		_push(PC);
		PC = level * 8;
	} else
		_irq_pending = level;
}

void z80::ei() {
	flags.I = 1;
	if (_irq_pending)
		raise(_irq_pending);
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

void z80::cb() {
	// FIXME
	halt();
}

void z80::dd() {
	// FIXME
	halt();
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

z80::z80(Memory &m, jmp_buf *jb, CPU::statfn s, PortDevice<z80> &d): CPU(m, jb, s)
{
	_ports = &d;
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
}
