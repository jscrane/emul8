/*
 * r6502.h
 */
#ifndef _R6502_H
#define _R6502_H

#ifdef __GNUG__
#pragma interface
#endif

#ifndef _MEMORY_H
#include <memory.h>
#endif

#ifndef _CPU_H
#include <cpu.h>
#endif

class r6502: public CPU {
public:
	void raise (int);
	void reset ();
	void run (unsigned);
	char *status ();

	r6502 (Memory &, jmp_buf *, CPU::statfn);
private:
	/* registers */
	byte S, A, X, Y;
	byte N, V, B, D, I, Z, C;
	union {
		struct {
			unsigned C:1;
			unsigned Z:1;
			unsigned I:1;
			unsigned D:1;
			unsigned B:1;
			unsigned _:1;		// unused
			unsigned V:1;
			unsigned N:1;
		} bits;
		byte value;
	} P;
	byte _toBCD[256], _fromBCD[256];	// BCD maps
	bool _irq;				// interrupt pending

	void irq ();
	void nmi ();
	byte flags();

	/* stack */
	inline void pusha (Memory::address ret) {
	  _mem[0x0100+S--] = ret >> 8;
	  _mem[0x0100+S--] = ret & 0xff;
	}

	inline void pushb (byte b) {
	  _mem[0x0100+S--] = b;
	}

	inline byte popb () {
	  return _mem[++S+0x0100];
	}

	inline Memory::address popa () {
	  byte b = popb ();
	  return ((popb () << 8) | b);
	}

	static const Memory::address nmivec = 0xfffa;
	static const Memory::address resvec = 0xfffc;
	static const Memory::address ibvec = 0xfffe;
	
	inline Memory::address vector(Memory::address v) {
	  return (_mem[v+1] << 8) | _mem[v];
	}

	/* operators */
	inline void _cmp (byte a) { Z=N=A-a; C=(A>=a); }
	inline void _cpx (byte a) { Z=N=X-a; C=(X>=a); }
	inline void _cpy (byte a) { Z=N=Y-a; C=(Y>=a); }
	inline void _and (byte a) { Z=N=A&=a; }
	inline void _eor (byte a) { Z=N=A^=a; }
	inline void _ora (byte a) { Z=N=A|=a; }
	inline void _lda (byte a) { Z=N=A=a; }
	inline void _ldx (byte a) { Z=N=X=a; }
	inline void _ldy (byte a) { Z=N=Y=a; }

	/* modes */
	inline Memory::address _a () { 
		Memory::address a = _mem[PC++];
		return a | (_mem[PC++] << 8);
	}
	inline Memory::address _ax () { return _a()+X; }
	inline Memory::address _ay () { return _a()+Y; }
	inline Memory::address _z () { return _mem[PC++]; }
	inline Memory::address _zx () { return (_z()+X) & 0xff; }
	inline Memory::address _zy () { return (_z()+Y) & 0xff; }
	inline Memory::address _i (Memory::address a) { 
		return (_mem[a+1]<<8)|_mem[a]; 
	}
	inline Memory::address _ix () { return _i(_zx()); }
	inline Memory::address _iy () { return _i(_mem[PC++])+Y; }

	void _adc (byte a);
	void _sbc (byte a) { if (P.bits.D) sbcd(a); else _adc(~a); }
	void sbcd (byte a);

	inline byte __ror (byte b) {
		N=b>>1; if (C) N|=0x80; C=b&1; return Z=N;
	}
	inline void _ror (Memory::address a) {
		_mem[a] = __ror(_mem[a]);
	}
	inline byte __rol (byte b) {
		N=b<<1; if (C) N|=1; C=(b&0x80)!=0; return Z=N;
	}
	inline void _rol (Memory::address a) {
		_mem[a] = __rol(_mem[a]);
	}
	inline byte __asl (byte b) { C=(b&0x80)!=0; return Z=N=b<<1; }
	inline void _asl (Memory::address a) {
		_mem[a] = __asl(_mem[a]);
	}
	inline byte __lsr (byte b) { C=b&1; Z=b>>1; N=0; return Z; }
	inline void _lsr (Memory::address a) {
		_mem[a] = __lsr(_mem[a]);
	}
	inline void _inc (Memory::address a) {
		Z=N=1+_mem[a]; _mem[a]=Z;
	}
	inline void _dec (Memory::address a) {
		Z=N=_mem[a]-1; _mem[a]=Z;
	}
	inline void _bit (byte z) { V=((z & 0x40)!=0); N=(z & 0x80); Z=(A & z); }
	inline void _bra() {
		byte b = _mem[PC];
		PC += b;
		if (b > 127) PC -= 0x0100;
	}

	/* dispatch table */
	typedef void (r6502::*OP)(); OP _ops[256];

	/* operations */
	void brk ();
	void ora_ix () { _ora (_mem[_ix()]); }
	void ill ();
	void nop2 () { PC++; }
	void ora_z () { _ora (_mem[_z()]); }
	void asl_z () { _asl (_z()); }
	void php ();
	void ora_ () { _ora (_mem[PC++]); }
	void asl () { C=(A&0x80)!=0; Z=N=A<<=1; }
	void nop3 () { PC+=2; }
	void ora_a () { _ora (_mem[_a()]); }
	void asl_a () { _asl (_a()); }
	// 10
	void bpl () { if (!(N & 0x80)) _bra(); PC++; }
	void ora_iy () { _ora (_mem[_iy()]); }
	void ora_zx () { _ora (_mem[_zx()]); }
	void asl_zx () { _asl (_zx()); }
	void clc () { C=0; }
	void ora_ay () { _ora (_mem[_ay()]); }
	void nop () { }
	void ora_ax () { _ora (_mem[_ax()]); }
	void asl_ax () { _asl (_ax()); }
	// 20
	void jsr ();
	void and_ix () { _and (_mem[_ix()]); }
	void bit_z () { _bit (_mem[_z()]); }
	void and_z () { _and (_mem[_z()]); }
	void rol_z () { _rol (_z()); }
	void plp ();
	void and_ () { _and (_mem[PC++]); }
	void rol () { A=__rol (A); }
	void bit_a () { _bit (_mem[_a()]); }
	void and_a () { _and (_mem[_a()]); }
	void rol_a () { _rol (_a()); }
	// 30
	void bmi () { if (N & 0x80) _bra(); PC++; }
	void and_iy () { _and (_mem[_iy()]); }
	void and_zx () { _and (_mem[_zx()]); }
	void rol_zx () { _rol (_zx()); }
	void sec () { C=1; }
	void and_ay () { _and (_mem[_ay()]); }
	void and_ax () { _and (_mem[_ax()]); }
	void rol_ax () { _rol (_ax()); }
	// 40
	void rti ();
	void eor_ix () { _eor (_mem[_ix()]); }
	void eor_z () { _eor (_mem[_z()]); }
	void lsr_z () { _lsr (_z()); }
	void pha () { pushb (A); }
	void eor_ () { _eor (_mem[PC++]); }
	void lsr_ () { A=__lsr(A); }
	void jmp () { PC = _a (); }
	void eor_a () { _eor (_mem[_a()]); }
	void lsr_a () { _lsr (_a()); }
	// 50
	void bvc () { if (!V) _bra(); PC++; }
	void eor_iy () { _eor (_mem[_iy()]); }
	void eor_zx () { _eor (_mem[_zx()]); }
	void lsr_zx () { _lsr (_zx()); }
	void cli () { P.bits.I = 0; }
	void eor_ay () { _eor (_mem[_ay()]); }
	void eor_ax () { _eor (_mem[_ax()]); }
	void lsr_ax () { _lsr (_ax()); }
	// 60
	void rts ();
	void adc_ix () { _adc (_mem[_ix()]); }
	void adc_z () { _adc (_mem[_z()]); }
	void ror_z () { _ror (_z()); }
	void pla () { Z=N=A=popb (); }
	void adc_ () { _adc (_mem[PC++]); }
	void ror_ () { A=__ror (A); }
	void jmp_i () { PC = _i(_a()); }
	void adc_a () { _adc (_mem[_a()]); }
	void ror_a () { _ror (_a()); }
	// 70
	void bvs () { if (V) _bra(); PC++; }
	void adc_iy () { _adc (_mem[_iy()]); }
	void adc_zx () { _adc (_mem[_zx()]); }
	void ror_zx () { _ror (_zx ()); }
	void sei () { P.bits.I = 1; }
	void adc_ay () { _adc (_mem[_ay()]); }
	void adc_ax () { _adc (_mem[_ax()]); }
	void ror_ax () { _ror (_ax ()); }
	// 80
	void sta_ix () { _mem[_ix()] = A; }
	void sty_z () { _mem[_z()] = Y; }
	void sta_z () { _mem[_z()] = A; }
	void stx_z () { _mem[_z()] = X; }
	void dey () { Z=N=--Y; }
	void txa () { Z=N=A=X; }
	void sty_a () { _mem[_a()] = Y; }
	void sta_a () { _mem[_a()] = A; }
	void stx_a () { _mem[_a()] = X; }
	// 90
	void bcc () { if (!C) _bra(); PC++; }
	void sta_iy () { _mem[_iy()] = A; }
	void sty_zx () { _mem[_zx()] = Y; }
	void sta_zx () { _mem[_zx()] = A; }
	void stx_zy () { _mem[_zy()] = X; }
	void tya () { Z=N=A=Y; }
	void sta_ay () { _mem[_ay()] = A; }
	void txs () { S=X; }
	void sta_ax () { _mem[_ax()] = A; }
	// a0
	void ldy_ () { _ldy (_mem[PC++]); }
	void lda_ix () { _lda (_mem[_ix()]); }
	void ldx_ () { _ldx (_mem[PC++]); }
	void lax_ix () { lda_ix (); X=A; }
	void ldy_z () { _ldy (_mem[_z()]); }
	void lda_z () { _lda (_mem[_z()]); }
	void ldx_z () { _ldx (_mem[_z()]); }
	void lax_z () { lda_z (); X=A; }
	void tay () { Z=N=Y=A; }
	void lda_ () { _lda (_mem[PC++]); }
	void tax () { Z=N=X=A; }
	void ldy_a () { _ldy (_mem[_a()]); }
	void lda_a () { _lda (_mem[_a()]); }
	void ldx_a () { _ldx (_mem[_a()]); }
	void lax_a () { lda_a (); X=A; }
	// b0
	void bcs () { if (C) _bra(); PC++; }
	void lda_iy () { _lda (_mem[_iy()]); }
	void lax_iy () { lda_iy (); X=A; }
	void ldy_zx () { _ldy (_mem[_zx()]); }
	void lda_zx () { _lda (_mem[_zx()]); }
	void ldx_zy () { _ldx (_mem[_zy()]); }
	void lax_zy () { ldx_zy (); A=X; }
	void clv () { V=0; }
	void lda_ay () { _lda (_mem[_ay()]); }
	void tsx () { Z=N=X=S; }
	void ldy_ax () { _ldy (_mem[_ax()]); }
	void lda_ax () { _lda (_mem[_ax()]); }
	void ldx_ay () { _ldx (_mem[_ay()]); }
	void lax_ay () { ldx_ay (); A=X; }
	// c0
	void cpy_ () { _cpy (_mem[PC++]); }
	void cmp_ix () { _cmp (_mem[_ix()]); }
	void cpy_z () { _cpy (_mem[_z()]); }
	void cmp_z () { _cmp (_mem[_z()]); }
	void dec_z () { _dec (_z()); }
	void iny () { Z=N=++Y; }
	void cmp_ () { _cmp (_mem[PC++]); }
	void dex () { Z=N=--X; }
	void cpy_a () { _cpy (_mem[_a()]); }
	void cmp_a () { _cmp (_mem[_a()]); }
	void dec_a () { _dec (_a()); }
	// d0
	void bne () { if (Z) _bra(); PC++; }
	void cmp_iy () { _cmp (_mem[_iy()]); }
	void cmp_zx () { _cmp (_mem[_zx()]); }
	void dec_zx () { _dec (_zx()); }
	void cld () { P.bits.D = 0; }
	void cmp_ay () { _cmp (_mem[_ay()]); }
	void cmp_ax () { _cmp (_mem[_ax()]); }
	void dec_ax () { _dec (_ax()); }
	// e0
	void cpx_ () { _cpx (_mem[PC++]); }
	void sbc_ix () { _sbc (_mem[_ix()]); }
	void cpx_z () { _cpx (_mem[_z()]); }
	void sbc_z () { _sbc (_mem[_z()]); }
	void inc_z () { _inc (_z()); }
	void inx () { Z=N=++X; }
	void sbc_ () { _sbc (_mem[PC++]); }
	void cpx_a () { _cpx (_mem[_a()]); }
	void sbc_a () { _sbc (_mem[_a()]); }
	void inc_a () { _inc (_a()); }
	// f0
	void beq () { if (!Z) _bra(); PC++; }
	void sbc_iy () { _sbc (_mem[_iy()]); }
	void sbc_zx () { _sbc (_mem[_zx()]); }
	void inc_zx () { _inc (_zx()); }
	void sed () { P.bits.D = 1; }
	void sbc_ay () { _sbc (_mem[_ay()]); }
	void sbc_ax () { _sbc (_mem[_ax()]); }
	void inc_ax () { _inc (_ax()); }
};
#endif
