#include <stdio.h>

#include "memory.h"
#include "ports.h"
#include "cpu.h"
#include "z80.h"

#define CPU_STATE_FMT "%04x %02x %02x %04x %04x %04x %04x %d%d%d%d%d%d%d%d\r\n",\
			PC, op, A, BC, DE, HL, SP, flags.S, flags.Z,\
			flags.I, flags.H, flags._, flags.P, flags.__, flags.C

void z80::step() {
	byte op = _mem[PC];
#if defined(CPU_DEBUG)
	if (_debug)
		_status(CPU_STATE_FMT);
#endif
	PC++;
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
	A = 0;
	_sr(0);
	BC = DE = HL = PC = SP = 0;
	_irq_pending = 0;
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
	if (flags.C || hi > 0x9 || (hi >= 0x9 && lo > 9)) {
		a |= 0x60;
		c = 1;
	}
	_add(a);
	flags.C = c;
}

void z80::hlt() {
	_status("CPU halted at %04x\r\n%s", (PC-1), status());
	longjmp(*_err, 1);
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

z80::i8080(Memory &m, jmp_buf *jb, CPU::statfn s, PortDevice &d): CPU(m, jb, s)
{
	_ports = &d;
	_debug = false;

	OP *p = _ops;

	// 0x
	*p++ = &z80::nop; *p++ = &z80::lxib; 
	*p++ = &z80::staxb; *p++ = &z80::inxb;
	*p++ = &z80::inrb; *p++ = &z80::dcrb;
	*p++ = &z80::mvib; *p++ = &z80::rlc;
	*p++ = &z80::nop; *p++ = &z80::dadb;
	*p++ = &z80::ldaxb; *p++ = &z80::dcxb;
	*p++ = &z80::inrc; *p++ = &z80::dcrc;
	*p++ = &z80::mvic; *p++ = &z80::rrc;

	// 1x
	*p++ = &z80::nop; *p++ = &z80::lxid; 
	*p++ = &z80::staxd; *p++ = &z80::inxd;
	*p++ = &z80::inrd; *p++ = &z80::dcrd;
	*p++ = &z80::mvid; *p++ = &z80::ral;
	*p++ = &z80::nop; *p++ = &z80::dadd;
	*p++ = &z80::ldaxd; *p++ = &z80::dcxd;
	*p++ = &z80::inre; *p++ = &z80::dcre;
	*p++ = &z80::mvie; *p++ = &z80::rar;

	// 2x
	*p++ = &z80::nop; *p++ = &z80::lxih;
	*p++ = &z80::shld; *p++ = &z80::inxh;
	*p++ = &z80::inrh; *p++ = &z80::dcrh;
	*p++ = &z80::mvih; *p++ = &z80::daa;
	*p++ = &z80::nop; *p++ = &z80::dadh; 
	*p++ = &z80::lhld; *p++ = &z80::dcxh; 
	*p++ = &z80::inrl; *p++ = &z80::dcrl; 
	*p++ = &z80::mvil; *p++ = &z80::cma; 

	// 3x
	*p++ = &z80::nop; *p++ = &z80::lxisp;
	*p++ = &z80::sta; *p++ = &z80::inxsp;
	*p++ = &z80::inrm; *p++ = &z80::dcrm;
	*p++ = &z80::mvim; *p++ = &z80::stc;
	*p++ = &z80::nop; *p++ = &z80::dadsp;
	*p++ = &z80::lda; *p++ = &z80::dcxsp;
	*p++ = &z80::inra; *p++ = &z80::dcra;
	*p++ = &z80::mvia; *p++ = &z80::cmc;

	// 4x
	*p++ = &z80::movbb; *p++ = &z80::movbc;
	*p++ = &z80::movbd; *p++ = &z80::movbe;
	*p++ = &z80::movbh; *p++ = &z80::movbl;
	*p++ = &z80::movbm; *p++ = &z80::movba;
	*p++ = &z80::movcb; *p++ = &z80::movcc;
	*p++ = &z80::movcd; *p++ = &z80::movce;
	*p++ = &z80::movch; *p++ = &z80::movcl;
	*p++ = &z80::movcm; *p++ = &z80::movca;

	// 5x
	*p++ = &z80::movdb; *p++ = &z80::movdc;
	*p++ = &z80::movdd; *p++ = &z80::movde;
	*p++ = &z80::movdh; *p++ = &z80::movdl;
	*p++ = &z80::movdm; *p++ = &z80::movda;
	*p++ = &z80::moveb; *p++ = &z80::movec;
	*p++ = &z80::moved; *p++ = &z80::movee;
	*p++ = &z80::moveh; *p++ = &z80::movel;
	*p++ = &z80::movem; *p++ = &z80::movea;

	// 6x
	*p++ = &z80::movhb; *p++ = &z80::movhc;
	*p++ = &z80::movhd; *p++ = &z80::movhe;
	*p++ = &z80::movhh; *p++ = &z80::movhl;
	*p++ = &z80::movhm; *p++ = &z80::movha;
	*p++ = &z80::movlb; *p++ = &z80::movlc;
	*p++ = &z80::movld; *p++ = &z80::movle;
	*p++ = &z80::movlh; *p++ = &z80::movll;
	*p++ = &z80::movlm; *p++ = &z80::movla;

	// 7x
	*p++ = &z80::movmb; *p++ = &z80::movmc;
	*p++ = &z80::movmd; *p++ = &z80::movme;
	*p++ = &z80::movmh; *p++ = &z80::movml;
	*p++ = &z80::hlt; *p++ = &z80::movma;
	*p++ = &z80::movab; *p++ = &z80::movac;
	*p++ = &z80::movad; *p++ = &z80::movae;
	*p++ = &z80::movah; *p++ = &z80::moval;
	*p++ = &z80::movam; *p++ = &z80::movaa;

	// 8x
	*p++ = &z80::addb; *p++ = &z80::addc;
	*p++ = &z80::addd; *p++ = &z80::adde;
	*p++ = &z80::addh; *p++ = &z80::addl;
	*p++ = &z80::addm; *p++ = &z80::adda;
	*p++ = &z80::adcb; *p++ = &z80::adcc;
	*p++ = &z80::adcd; *p++ = &z80::adce;
	*p++ = &z80::adch; *p++ = &z80::adcl;
	*p++ = &z80::adcm; *p++ = &z80::adca;

	// 9x
	*p++ = &z80::subb; *p++ = &z80::subc;
	*p++ = &z80::subd; *p++ = &z80::sube;
	*p++ = &z80::subh; *p++ = &z80::subl;
	*p++ = &z80::subm; *p++ = &z80::suba;
	*p++ = &z80::sbbb; *p++ = &z80::sbbc;
	*p++ = &z80::sbbd; *p++ = &z80::sbbe;
	*p++ = &z80::sbbh; *p++ = &z80::sbbl;
	*p++ = &z80::sbbm; *p++ = &z80::sbba;

	// Ax
	*p++ = &z80::anab; *p++ = &z80::anac;
	*p++ = &z80::anad; *p++ = &z80::anae;
	*p++ = &z80::anah; *p++ = &z80::anal;
	*p++ = &z80::anam; *p++ = &z80::anaa;
	*p++ = &z80::xrab; *p++ = &z80::xrac;
	*p++ = &z80::xrad; *p++ = &z80::xrae;
	*p++ = &z80::xrah; *p++ = &z80::xral;
	*p++ = &z80::xram; *p++ = &z80::xraa;

	// Bx
	*p++ = &z80::orab; *p++ = &z80::orac;
	*p++ = &z80::orad; *p++ = &z80::orae;
	*p++ = &z80::orah; *p++ = &z80::oral;
	*p++ = &z80::oram; *p++ = &z80::oraa;
	*p++ = &z80::cmpb; *p++ = &z80::cmpc;
	*p++ = &z80::cmpd; *p++ = &z80::cmpe;
	*p++ = &z80::cmph; *p++ = &z80::cmpl;
	*p++ = &z80::cmpm; *p++ = &z80::cmpa;

	// Cx
	*p++ = &z80::rnz; *p++ = &z80::popb;
	*p++ = &z80::jnz; *p++ = &z80::jmp;
	*p++ = &z80::cnz; *p++ = &z80::pushb;
	*p++ = &z80::adi; *p++ = &z80::rst0;
	*p++ = &z80::rz; *p++ = &z80::ret;
	*p++ = &z80::jz; *p++ = &z80::jmp;
	*p++ = &z80::cz; *p++ = &z80::call;
	*p++ = &z80::aci; *p++ = &z80::rst1;

	// Dx
	*p++ = &z80::rnc; *p++ = &z80::popd;
	*p++ = &z80::jnc; *p++ = &z80::out;
	*p++ = &z80::cnc; *p++ = &z80::pushd;
	*p++ = &z80::sui; *p++ = &z80::rst2;
	*p++ = &z80::rc; *p++ = &z80::ret;
	*p++ = &z80::jc; *p++ = &z80::in;
	*p++ = &z80::cc; *p++ = &z80::call;
	*p++ = &z80::sbi; *p++ = &z80::rst3;

	// Ex
	*p++ = &z80::rpo; *p++ = &z80::poph;
	*p++ = &z80::jpo; *p++ = &z80::xthl;
	*p++ = &z80::cpo; *p++ = &z80::pushh;
	*p++ = &z80::ani; *p++ = &z80::rst4;
	*p++ = &z80::rpe; *p++ = &z80::pchl;
	*p++ = &z80::jpe; *p++ = &z80::xchg;
	*p++ = &z80::cpe; *p++ = &z80::call;
	*p++ = &z80::xri; *p++ = &z80::rst5;

	// Fx
	*p++ = &z80::rp; *p++ = &z80::pop;
	*p++ = &z80::jp; *p++ = &z80::di;
	*p++ = &z80::cp; *p++ = &z80::push;
	*p++ = &z80::ori; *p++ = &z80::rst6;
	*p++ = &z80::rm; *p++ = &z80::sphl;
	*p++ = &z80::jm; *p++ = &z80::ei;
	*p++ = &z80::cm; *p++ = &z80::call;
	*p++ = &z80::cpi; *p++ = &z80::rst7;
}
