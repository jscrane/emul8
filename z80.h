#ifndef __Z80_H__
#define __Z80_H__

#undef sbi
#undef and

class z80: public CPU {
public:
	z80(Memory &, jmp_buf *, CPU::statfn, PortDevice<z80> &);

	void run(unsigned);
	void reset();
	void raise(int);
	char *status();

	inline byte a() { return A; }
	inline byte b() { return B; }
	inline byte c() { return C; }
	inline byte d() { return D; }
	inline byte e() { return E; }
	inline byte h() { return H; }
	inline byte l() { return L; }
	inline word af() { return AF; }
	inline word bc() { return BC; }
	inline word de() { return DE; }
	inline word hl() { return HL; }
	inline byte sr() { return F; }
	inline byte r() { return R; }
	inline word af_() { return AF_; }
	inline word bc_() { return BC_; }
	inline word de_() { return DE_; }
	inline word hl_() { return HL_; }
	inline word ix() { return IX; }
	inline word iy() { return IY; }
	inline word sp() { return SP; }
	inline word pc() { return PC; }
	inline bool halted() { return _halted; }

	inline void af(word w) { AF = w; }
	inline void bc(word w) { BC = w; }
	inline void de(word w) { DE = w; }
	inline void hl(word w) { HL = w; }
	inline void pc(word w) { PC = w; }
	inline void sp(word w) { SP = w; }
	inline void ix(word w) { IX = w; }
	inline void iy(word w) { IY = w; }
	inline void af_(word w) { AF_ = w; }
	inline void bc_(word w) { BC_ = w; }
	inline void de_(word w) { DE_ = w; }
	inline void hl_(word w) { HL_ = w; }

	inline unsigned long ts() { return _ts; }
	inline void ts(int t) { _ts += t; }

private:
	inline void step();

	union {
		struct {
			union {
				struct __attribute__((packed)) {
					unsigned C:1;
					unsigned N:1;
					unsigned P:1;
					unsigned _:1;	// always 0
					unsigned H:1;
					unsigned I:1;
					unsigned Z:1;
					unsigned S:1;
				} flags;
				byte F;
			};
			byte A;
		};
		word AF;
	};
	static const byte flag35 = (1 << 3) | (1 << 5);

	union {
		struct { byte C, B; };
		word BC;
	};
	union {
		struct { byte E, D; };
		word DE;
	};
	union {
		struct { byte L, H; };
		word HL;
	};

	Memory::address SP;

	word AF_, BC_, DE_, HL_, IX, IY;

	union {
		struct { byte R, I; };
		word IR;
	};

	unsigned long _ts;
	bool _halted;

	int _irq_pending;
	PortDevice<z80> *_ports;

	typedef void (z80::*OP)(); 
	OP _ops[256];
	OP _cb[256];

	static int parity_table[256];

	inline byte _rb(Memory::address a) {
#if defined(CPU_DEBUG)
		printf("%5d MC %04x\n", _ts, a);
#endif
		ts(3);
#if defined(CPU_DEBUG)
		printf("%5d MR %04x %02x\n", _ts, a, (byte)_mem[a]);
#endif
		return _mem[a];
	}

	inline byte _sb(Memory::address a, byte b) {
#if defined(CPU_DEBUG)
		printf("%5d MC %04x\n", _ts, a);
#endif
		ts(3);
#if defined(CPU_DEBUG)
		printf("%5d MW %04x %02x\n", _ts, a, b);
#endif
		_mem[a] = b;
	}

	inline void _mc(Memory::address a, int i) {
#if defined(CPU_DEBUG)
		printf("%5d MC %04x\n", _ts, a);
#endif
		ts(i);
	}

	inline void _pc(byte port, int i) {
#if defined(CPU_DEBUG)
		printf("%5d PC %04x\n", _ts, port);
#endif
		ts(i);
	}

	inline word _rw(Memory::address a) { 
		return _rb(a) + (_rb(a+1) << 8); 
	}

	inline void _sw(Memory::address a, word w) {
		_sb(a+1, w >> 8);
		_sb(a, w & 0xff);
	}

	inline void _sz(byte r) {
		flags.S = ((r & 0x80) != 0);
		flags.Z = (r == 0);
	}

	inline void _szp(byte r) {
		_sz(r);
		flags.P = parity_table[r];
	}

	inline void _szp35(byte r) {
		_sz(r);
		flags.P = parity_table[r];
		F |= (r & flag35);
	}

	inline void _inc(byte &b) {
		word w = b + 1;
		byte r = w & 0xff;
		_sz(r);
		flags.P = r == 0x80;
		flags.N = 0;
		flags.H = !(r & 0x0f);
		b = r;
	}

	inline void _dec(byte &b) {
		word w = b - 1;
		byte r = w & 0xff;
		_sz(r);
		flags.P = r == 0x7f;
		flags.N = 1;
		flags.H = !(b & 0x0f);
		b = r;
	}

	inline void _dad(word w) {
		_mc(IR, 1); _mc(IR, 1); _mc(IR, 1);
		_mc(IR, 1); _mc(IR, 1); _mc(IR, 1); _mc(IR, 1);
		unsigned long r = HL + w;
		HL = (r & 0xffff);
		flags.C = (r > 0xffff);
		flags.H = 1;
	}

	inline void _exch(word &a, word &b) { word t = b; b = a; a = t; }

	inline word _pop() { word w = _rw(SP); SP += 2; return w; }
	inline void _push(word w) { SP -= 2; _sw(SP, w); }

	inline void _jmp(byte c) { if (c) jp(); else { _mc(PC, 3); _mc(PC+1, 3); PC += 2; } }
	inline void _ret(byte c) { _mc(IR, 1); if (c) ret(); }
	inline void _call(byte c) { if (c) call(); else { _mc(PC, 3); _mc(PC+1, 3); PC += 2; } }
	inline void _jr(byte c) { if (c) jr(); else { _mc(PC, 3); PC++; } }

	// 0x00
	void nop() {}
	void ldbcpc() { BC = _rw(PC); PC += 2; }
	void ldBCa() { _sb(BC, A); }
	void incbc() { BC++; _mc(IR, 1); _mc(IR, 1); }
	void incb() { _inc(B); }
	void decb() { _dec(B); }
	void ldb() { B = _rb(PC++); }
	void rlca() { flags.C = ((A & 0x80) >> 7); A = (A << 1) | flags.C; }

	// 0x08
	void exaf() { _exch(AF, AF_); }
	void addhlbc() { _dad(BC); }
	void ldaBC() { A = _rb(BC); }
	void decbc() { BC--; _mc(IR, 1); _mc(IR, 1); }
	void incc() { _inc(C); }
	void decc() { _dec(C); }
	void ldc() { C = _rb(PC++); }
	void rrca() { flags.C = (A & 0x01); A = (A >> 1) | (flags.C << 7); }

	// 0x10
	void djnz() { _mc(IR, 1); _jr(--B); }
	void lddepc() { DE = _rw(PC); PC += 2; }
	void ldDEa() { _sb(DE, A); }
	void incde() { DE++; _mc(IR, 1); _mc(IR, 1); }
	void incd() { _inc(D); }
	void decd() { _dec(D); }
	void ldd() { D = _rb(PC++); }
	void rla() { 
		byte b = (A << 1) | flags.C;
		flags.C = (A & 0x80) >> 7;
		A = b;
	}

	// 0x18
	void jr() { char o = (char)_rb(PC); _mc(PC, 1); _mc(PC, 1); _mc(PC, 1); _mc(PC, 1); _mc(PC, 1); PC += o + 1; }
	void addhlde() { _dad(DE); }
	void ldaDE() { A = _rb(DE); }
	void decde() { DE--; _mc(IR, 1); _mc(IR, 1); }
	void ince() { _inc(E); }
	void dece() { _dec(E); }
	void lde() { E = _rb(PC++); }
	void rra() {
		byte b = (A >> 1) | (flags.C << 7);
		flags.C = (A & 1);
		A = b;
	}

	// 0x20
	void jrnz() { _jr(!flags.Z); }
	void ldhlpc() { HL = _rw(PC); PC += 2; }
	void ldPChl() { _sw(_rw(PC), HL); PC += 2; }
	void inchl() { HL++; _mc(IR, 1); _mc(IR, 1); }
	void inch() { _inc(H); }
	void dech() { _dec(H); }
	void ldh() { H = _rb(PC++); }
	void daa();

	// 0x28
	void jrz() { _jr(flags.Z); }
	void addhlhl() { _dad(HL); }
	void ldhlPC() { HL = _rw(_rw(PC)); PC += 2; }
	void dechl() { HL--; _mc(IR, 1); _mc(IR, 1); }
	void incl() { _inc(L); }
	void decl() { _dec(L); }
	void ldl() { L = _rb(PC++); }
	void cpl() { A = ~A; flags.H = flags.N = 1; }

	// 0x30
	void jrnc() { _jr(!flags.C); }
	void ldsppc() { SP = _rw(PC); PC += 2; }
	void ldPCa() { _sb(_rw(PC), A); PC += 2; }
	void incsp() { SP++; _mc(IR, 1); _mc(IR, 1); }
	void incHL() { byte b = _rb(HL); _mc(HL, 1); _inc(b); _sb(HL, b); }
	void decHL() { byte b = _rb(HL); _mc(HL, 1); _dec(b); _sb(HL, b); }
	void ldHL() { _sb(HL, _rb(PC++)); }
	void scf() { flags.C = 1; flags.N = flags.H = 0; }

	// 0x38
	void jrc() { _jr(flags.C); }
	void addhlsp() { _dad(SP); }
	void ldaPC() { A = _rb(_rw(PC)); PC += 2; }
	void decsp() { SP--; _mc(IR, 1); _mc(IR, 1); }
	void inca() { _inc(A); }
	void deca() { _dec(A); }
	void lda() { A = _rb(PC++); }
	void ccf() { flags.C = !flags.C; flags.N = 0; }

	// 0x40
	void ldbb() {}
	void ldbc() { B = C; }
	void ldbd() { B = D; }
	void ldbe() { B = E; }
	void ldbh() { B = H; }
	void ldbl() { B = L; }
	void ldbHL() { B = _rb(HL); }
	void ldba() { B = A; }

	// 0x48
	void ldcb() { C = B; }
	void ldcc() {}
	void ldcd() { C = D; }
	void ldce() { C = E; }
	void ldch() { C = H; }
	void ldcl() { C = L; }
	void ldcHL() { C = _rb(HL); }
	void ldca() { C = A; }

	// 0x50
	void lddb() { D = B; }
	void lddc() { D = C; }
	void lddd() {}
	void ldde() { D = E; }
	void lddh() { D = H; }
	void lddl() { D = L; }
	void lddHL() { D = _rb(HL); }
	void ldda() { D = A; }

	// 0x58
	void ldeb() { E = B; }
	void ldec() { E = C; }
	void lded() { E = D; }
	void ldee() {}
	void ldeh() { E = H; }
	void ldel() { E = L; }
	void ldeHL() { E = _rb(HL); }
	void ldea() { E = A; }

	// 0x60
	void ldhb() { H = B; }
	void ldhc() { H = C; }
	void ldhd() { H = D; }
	void ldhe() { H = E; }
	void ldhh() {}
	void ldhl() { H = L; }
	void ldhHL() { H = _rb(HL); }
	void ldha() { H = A; }

	// 0x68
	void ldlb() { L = B; }
	void ldlc() { L = C; }
	void ldld() { L = D; }
	void ldle() { L = E; }
	void ldlh() { L = H; }
	void ldll() {}
	void ldlHL() { L = _rb(HL); }
	void ldla() { L = A; }

	// 0x70
	void ldHLb() { _sb(HL, B); }
	void ldHLc() { _sb(HL, C); }
	void ldHLd() { _sb(HL, D); }
	void ldHLe() { _sb(HL, E); }
	void ldHLh() { _sb(HL, H); }
	void ldHLl() { _sb(HL, L); }
	void halt() { _halted = true; PC--; }
	void ldHLa() { _sb(HL, A); }

	// 0x78
	void ldab() { A = B; }
	void ldac() { A = C; }
	void ldad() { A = D; }
	void ldae() { A = E; }
	void ldah() { A = H; }
	void ldal() { A = L; }
	void ldaHL() { A = _rb(HL); }
	void ldaa() {}

	// 0x80
	inline void _add(byte x) {
		word w = A + x;
		byte b = A;
		A = w & 0xff;
		_sz(A);
		flags.C = w > 0xff;
		flags.N = 0;
		byte v = b ^ A ^ x;
		flags.P = (v >> 7) ^ flags.C;
		flags.H = (v >> 4) & 1;
	}
	void addab() { _add(B); }
	void addac() { _add(C); }
	void addad() { _add(D); }
	void addae() { _add(E); }
	void addah() { _add(H); }
	void addal() { _add(L); }
	void addaHL() { _add(_rb(HL)); }
	void addaa() { _add(A); }

	// 0x88
	inline void _adc(byte x) {
		word w = A + x + flags.C;
		byte b = A;
		A = w & 0xff;
		_sz(A);
		flags.C = w > 0xff;
		flags.N = 0;
		byte v = b ^ A ^ x;
		flags.P = (v >> 7) ^ flags.C;
		flags.H = (v >> 4) & 1;
	}
	void adcab() { _adc(B); }
	void adcac() { _adc(C); }
	void adcad() { _adc(D); }
	void adcae() { _adc(E); }
	void adcah() { _adc(H); }
	void adcal() { _adc(L); }
	void adcaHL() { _adc(_rb(HL)); }
	void adcaa() { _adc(A); }

	// 0x90
	inline void _sub(byte x) {
		byte b = A;
		flags.C = !flags.C;
		_adc(~x);
		flags.C = !flags.C;
		flags.N = 1;
		flags.H = (b & 0x0f) < (x & 0x0f);
	}
	void subab() { _sub(B); }
	void subac() { _sub(C); }
	void subad() { _sub(D); }
	void subae() { _sub(E); }
	void subah() { _sub(H); }
	void subal() { _sub(L); }
	void subaHL() { _sub(_rb(HL)); }
	void subaa() { _sub(A); }

	// 0x98
	inline void _sbc(byte x) {
		byte b = A;
		flags.C = !flags.C;
		_adc(~x);
		flags.C = !flags.C;
		flags.N = 1;
		flags.H = (b & 0x0f) < (x & 0x0f);
	}
	void sbcab() { _sbc(B); }
	void sbcac() { _sbc(C); }
	void sbcad() { _sbc(D); }
	void sbcae() { _sbc(E); }
	void sbcah() { _sbc(H); }
	void sbcal() { _sbc(L); }
	void sbcaHL() { _sbc(_rb(HL)); }
	void sbcaa() { _sbc(A); }

	// 0xa0
	inline void _and(byte b) {
		A &= b;
		_szp(A);
		flags.C = flags.N = 0;
		flags.H = 1;
	}
	void andb() { _and(B); }
	void andc() { _and(C); }
	void andd() { _and(D); }
	void ande() { _and(E); }
	void andh() { _and(H); }
	void andl() { _and(L); }
	void andHL() { _and(_rb(HL)); }
	void anda() { _and(A); }

	// 0xa8
	inline void _xor(byte b) {
		A ^= b;
		_szp(A);
		flags.C = flags.N = flags.H = 0;
	}
	void xorb() { _xor(B); }
	void xorc() { _xor(C); }
	void xord() { _xor(D); }
	void xore() { _xor(E); }
	void xorh() { _xor(H); }
	void xorl() { _xor(L); }
	void xorHL() { _xor(_rb(HL)); }
	void xora() { _xor(A); }

	// 0xb0
	inline void _or(byte b) {
		A |= b;
		_szp(A);
		flags.C = flags.N = flags.H = 0;
	}
	void orb() { _or(B); }
	void orc() { _or(C); }
	void ord() { _or(D); }
	void ore() { _or(E); }
	void orh() { _or(H); }
	void orl() { _or(L); }
	void orHL() { _or(_rb(HL)); }
	void ora() { _or(A); }

	// 0xb8
	inline void _cmp(byte b) {
		byte a = A;
		_sub(b);
		A = a;
	}
	void cpb() { _cmp(B); }
	void cpc() { _cmp(C); }
	void cpd() { _cmp(D); }
	void cpe() { _cmp(E); }
	void cph() { _cmp(H); }
	void cpL() { _cmp(L); }
	void cpHL() { _cmp(_rb(HL)); }
	void cpa() { _cmp(A); }

	// 0xc0
	void retnz() { _ret(!flags.Z); }
	void popbc() { BC = _pop(); }
	void jpnz() { _jmp(!flags.Z); }
	void jp() { PC = _rw(PC); }
	void callnz() { _call(!flags.Z); }
	void pushbc() { _mc(IR, 1); _push(BC); }
	void adda() { _add(_rb(PC++)); }
	void rst00() { _mc(IR, 1); _push(PC); PC = 0x00; }

	// 0xc8
	void retz() { _ret(flags.Z); }
	void ret() { PC = _pop(); }
	void jpz() { _jmp(flags.Z); }
	void cb();
	void callz() { _call(flags.Z); }
	void call() { word pc = _rw(PC); _mc(PC, 1); _push(PC+2); PC = pc; }
	void adca() { _adc(_rb(PC++)); }
	void rst08() { _mc(IR, 1); _push(PC); PC = 0x08; }

	// 0xd0
	void retnc() { _ret(!flags.C); }
	void popde() { DE = _pop(); }
	void jpnc() { _jmp(!flags.C); }
	void outa() { byte p = _rb(PC++); _pc(p, 1); _ports->out(p, A, this); _pc(p, 3); }
	void callnc() { _call(!flags.C); }
	void pushde() { _mc(IR, 1); _push(DE); }
	void suba() { _sub(_rb(PC++)); }
	void rst10() { _mc(IR, 1); _push(PC); PC = 0x10; }

	// 0xd8
	void retc() { _ret(flags.C); }
	void exx() { _exch(BC, BC_); _exch(DE, DE_); _exch(HL, HL_); }
	void jpc() { _jmp(flags.C); }
	void ina() { byte p = _rb(PC++); _pc(p, 1); A = _ports->in(p, this); _pc(p, 3); }
	void callc() { _call(flags.C); }
	void dd();
	void sbca() { _sbc(_rb(PC++)); }
	void rst18() { _mc(IR, 1); _push(PC); PC = 0x18; }

	// 0xe0
	void retpo() { _ret(!flags.P); }
	void pophl() { HL = _pop(); }
	void jppo() { _jmp(!flags.P); }
	void exSPhl() { word w = _pop(); _mc(SP, 1); _push(HL); HL = w; _mc(SP, 1); _mc(SP, 1); }
	void callpo() { _call(!flags.P); }
	void pushhl() { _mc(IR, 1); _push(HL); }
	void and() { _and(_rb(PC++)); }
	void rst20() { _mc(IR, 1); _push(PC); PC = 0x20; }

	// 0xe8
	void retpe() { _ret(flags.P); }
	void jphl() { PC = HL; }
	void jppe() { _jmp(flags.P); }
	void exdehl() { _exch(DE, HL); }
	void callpe() { _call(flags.P); }
	void ed();
	void xor() { _xor(_rb(PC++)); }
	void rst28() { _mc(IR, 1); _push(PC); PC = 0x28; }

	// 0xf0
	void retp() { _ret(!flags.S); }
	void popaf() { AF = _pop(); }
	void jpp() { _jmp(!flags.S); }
	void di() { flags.I = 0; }
	void callp() { _call(!flags.S); }
	void pushaf() { _mc(IR, 1); _push(AF); }
	void or() { _or(_rb(PC++)); }
	void rst30() { _mc(IR, 1); _push(PC); PC = 0x30; }

	// 0xf8
	void retm() { _ret(flags.S); }
	void ldsphl() { _mc(IR, 1); _mc(IR, 1); SP = HL; }
	void jpm() { _jmp(flags.S); }
	void ei();
	void callm() { _call(flags.S); }
	void fd();
	void cp() { _cmp(_rb(PC++)); }
	void rst38() { _mc(IR, 1); _push(PC); PC = 0x38; }

	// 0xCB extended instructions

	// 0x00
	inline void _rlc(byte &b) {
		b = (b << 1) | (b >> 7);
		flags.C = b & 0x01;
		_szp35(b);
	}

	void rlcB() { _rlc(B); }
	void rlcC() { _rlc(C); }
	void rlcD() { _rlc(D); }
	void rlcE() { _rlc(E); }
	void rlcH() { _rlc(H); }
	void rlcL() { _rlc(L); }
	void rlcHL() { byte b = _rb(HL); _mc(HL, 1); _rlc(b); _sb(HL, b); }
	void rlcA() { _rlc(A); }

	// 0x08
	inline void _rrc(byte &b) {
		flags.C = b & 0x01;
		b = (b >> 1) | (b << 7);
		_szp35(b);
	}

	void rrcB() { _rrc(B); }
	void rrcC() { _rrc(C); }
	void rrcD() { _rrc(D); }
	void rrcE() { _rrc(E); }
	void rrcH() { _rrc(H); }
	void rrcL() { _rrc(L); }
	void rrcHL() { byte b = _rb(HL); _mc(HL, 1); _rrc(b); _sb(HL, b); }
	void rrcA() { _rrc(A); }

	// 0x10
	inline void _rl(byte &b) {
		byte a = b;
		b = (b << 1) | flags.C;
		flags.C = (a >> 7);
		_szp35(b);
	}

	void rlB() { _rl(B); }
	void rlC() { _rl(C); }
	void rlD() { _rl(D); }
	void rlE() { _rl(E); }
	void rlH() { _rl(H); }
	void rlL() { _rl(L); }
	void rlHL() { byte b = _rb(HL); _mc(HL, 1); _rl(b); _sb(HL, b); }
	void rlA() { _rl(A); }

	// 0x18
	inline void _rr(byte &b) {
		byte a = b;
		b >>= 1;
		if (flags.C) b |= 0x80;
		flags.C = a & 0x01;
		_szp35(b);
	}

	void rrB() { _rr(B); }
	void rrC() { _rr(C); }
	void rrD() { _rr(D); }
	void rrE() { _rr(E); }
	void rrH() { _rr(H); }
	void rrL() { _rr(L); }
	void rrHL() { byte b = _rb(HL); _mc(HL, 1); _rr(b); _sb(HL, b); }
	void rrA() { _rr(A); }

	// 0x20
	inline void _sla(byte &b) {
		if (b & 0x80) flags.C = 1;
		b <<= 1;
		_szp35(b);
	}

	void slab() { _sla(B); }
	void slac() { _sla(C); }
	void slad() { _sla(D); }
	void slae() { _sla(E); }
	void slah() { _sla(H); }
	void slal() { _sla(L); }
	void slaHL() { byte b = _rb(HL); _mc(HL, 1); _sla(b); _sb(HL, b); }
	void slaa() { _sla(A); }

	// 0x28
	inline void _sra(byte &b) {
		flags.C = b & 0x01;
		b = (b & 0x80) | (b >> 1);
		_szp35(b);
	}

	void srab() { _sra(B); }
	void srac() { _sra(C); }
	void srad() { _sra(D); }
	void srae() { _sra(E); }
	void srah() { _sra(H); }
	void sral() { _sra(L); }
	void sraHL() { byte b = _rb(HL); _mc(HL, 1); _sra(b); _sb(HL, b); }
	void sraa() { _sra(A); }

	// 0x30
	inline void _sll(byte &b) {
		if (b & 0x80) flags.C = 1;
		b = (b << 1) | 0x01;
		_szp35(b);
	}

	void sllb() { _sll(B); }
	void sllc() { _sll(C); }
	void slld() { _sll(D); }
	void slle() { _sll(E); }
	void sllh() { _sll(H); }
	void slll() { _sll(L); }
	void sllHL() { byte b = _rb(HL); _mc(HL, 1); _sll(b); _sb(HL, b); }
	void slla() { _sll(A); }

	// 0x38
	inline void _srl(byte &b) {
		flags.C = b & 0x01;
		b >>= 1;
		_szp35(b);
	}

	void srlb() { _srl(B); }
	void srlc() { _srl(C); }
	void srld() { _srl(D); }
	void srle() { _srl(E); }
	void srlh() { _srl(H); }
	void srll() { _srl(L); }
	void srlHL() { byte b = _rb(HL); _mc(HL, 1); _srl(b); _sb(HL, b); }
	void srla() { _srl(A); }

	// 0x40
	inline void _bit(int i, byte b) {
		if (!(b & (1 << i)))
			flags.P = flags.Z = 1;
		if (i == 7 && (b & 0x80))
			flags.S = 1;
		F |= b & flag35;
		flags.H = 1;
	}

	inline void _bitHL(int i) {
		byte b = _rb(HL); _mc(HL, 1); _bit(i, b);
	}

	void bit0b() { _bit(0, B); }
	void bit0c() { _bit(0, C); }
	void bit0d() { _bit(0, D); }
	void bit0e() { _bit(0, E); }
	void bit0h() { _bit(0, H); }
	void bit0l() { _bit(0, L); }
	void bit0HL() { _bitHL(0); }
	void bit0a() { _bit(0, A); }

	// 0x48
	void bit1b() { _bit(1, B); }
	void bit1c() { _bit(1, C); }
	void bit1d() { _bit(1, D); }
	void bit1e() { _bit(1, E); }
	void bit1h() { _bit(1, H); }
	void bit1l() { _bit(1, L); }
	void bit1HL() { _bitHL(1); }
	void bit1a() { _bit(1, A); }

	// 0x50
	void bit2b() { _bit(2, B); }
	void bit2c() { _bit(2, C); }
	void bit2d() { _bit(2, D); }
	void bit2e() { _bit(2, E); }
	void bit2h() { _bit(2, H); }
	void bit2l() { _bit(2, L); }
	void bit2HL() { _bitHL(2); }
	void bit2a() { _bit(2, A); }

	// 0x58
	void bit3b() { _bit(3, B); }
	void bit3c() { _bit(3, C); }
	void bit3d() { _bit(3, D); }
	void bit3e() { _bit(3, E); }
	void bit3h() { _bit(3, H); }
	void bit3l() { _bit(3, L); }
	void bit3HL() { _bitHL(3); }
	void bit3a() { _bit(3, A); }

	// 0x60
	void bit4b() { _bit(4, B); }
	void bit4c() { _bit(4, C); }
	void bit4d() { _bit(4, D); }
	void bit4e() { _bit(4, E); }
	void bit4h() { _bit(4, H); }
	void bit4l() { _bit(4, L); }
	void bit4HL() { _bitHL(4); }
	void bit4a() { _bit(4, A); }

	// 0x68
	void bit5b() { _bit(5, B); }
	void bit5c() { _bit(5, C); }
	void bit5d() { _bit(5, D); }
	void bit5e() { _bit(5, E); }
	void bit5h() { _bit(5, H); }
	void bit5l() { _bit(5, L); }
	void bit5HL() { _bitHL(5); }
	void bit5a() { _bit(5, A); }

	// 0x70
	void bit6b() { _bit(6, B); }
	void bit6c() { _bit(6, C); }
	void bit6d() { _bit(6, D); }
	void bit6e() { _bit(6, E); }
	void bit6h() { _bit(6, H); }
	void bit6l() { _bit(6, L); }
	void bit6HL() { _bitHL(6); }
	void bit6a() { _bit(6, A); }

	// 0x78
	void bit7b() { _bit(7, B); }
	void bit7c() { _bit(7, C); }
	void bit7d() { _bit(7, D); }
	void bit7e() { _bit(7, E); }
	void bit7h() { _bit(7, H); }
	void bit7l() { _bit(7, L); }
	void bit7HL() { _bitHL(7); }
	void bit7a() { _bit(7, A); }

	// 0x80
	inline void _resHL(byte m) {
		byte b = _rb(HL);
		_mc(HL, 1);
		_sb(HL, b & m);
	}
	
	void res0b() { B &= 0xfe; }
	void res0c() { C &= 0xfe; }
	void res0d() { D &= 0xfe; }
	void res0e() { E &= 0xfe; }
	void res0h() { H &= 0xfe; }
	void res0l() { L &= 0xfe; }
	void res0HL() { _resHL(0xfe); }
	void res0a() { A &= 0xfe; }

	// 0x88
	void res1b() { B &= 0xfd; }
	void res1c() { C &= 0xfd; }
	void res1d() { D &= 0xfd; }
	void res1e() { E &= 0xfd; }
	void res1h() { H &= 0xfd; }
	void res1l() { L &= 0xfd; }
	void res1HL() { _resHL(0xfd); }
	void res1a() { A &= 0xfd; }

	// 0x90
	void res2b() { B &= 0xfb; }
	void res2c() { C &= 0xfb; }
	void res2d() { D &= 0xfb; }
	void res2e() { E &= 0xfb; }
	void res2h() { H &= 0xfb; }
	void res2l() { L &= 0xfb; }
	void res2HL() { _resHL(0xfb); }
	void res2a() { A &= 0xfb; }

	// 0x98
	void res3b() { B &= 0xf7; }
	void res3c() { C &= 0xf7; }
	void res3d() { D &= 0xf7; }
	void res3e() { E &= 0xf7; }
	void res3h() { H &= 0xf7; }
	void res3l() { L &= 0xf7; }
	void res3HL() { _resHL(0xf7); }
	void res3a() { A &= 0xf7; }

	// 0xa0
	void res4b() { B &= 0xef; }
	void res4c() { C &= 0xef; }
	void res4d() { D &= 0xef; }
	void res4e() { E &= 0xef; }
	void res4h() { H &= 0xef; }
	void res4l() { L &= 0xef; }
	void res4HL() { _resHL(0xef); }
	void res4a() { A &= 0xef; }

	// 0xa8
	void res5b() { B &= 0xdf; }
	void res5c() { C &= 0xdf; }
	void res5d() { D &= 0xdf; }
	void res5e() { E &= 0xdf; }
	void res5h() { H &= 0xdf; }
	void res5l() { L &= 0xdf; }
	void res5HL() { _resHL(0xdf); }
	void res5a() { A &= 0xdf; }

	// 0xb0
	void res6b() { B &= 0xbf; }
	void res6c() { C &= 0xbf; }
	void res6d() { D &= 0xbf; }
	void res6e() { E &= 0xbf; }
	void res6h() { H &= 0xbf; }
	void res6l() { L &= 0xbf; }
	void res6HL() { _resHL(0xbf); }
	void res6a() { A &= 0xbf; }

	// 0xb8
	void res7b() { B &= 0x7f; }
	void res7c() { C &= 0x7f; }
	void res7d() { D &= 0x7f; }
	void res7e() { E &= 0x7f; }
	void res7h() { H &= 0x7f; }
	void res7l() { L &= 0x7f; }
	void res7HL() { _resHL(0x7f); }
	void res7a() { A &= 0x7f; }

	// 0xc0
	inline void _setHL(byte m) {
		byte b = _rb(HL);
		_mc(HL, 1);
		_sb(HL, b | m);
	}

	void set0b() { B |= 0x01; }
	void set0c() { C |= 0x01; }
	void set0d() { D |= 0x01; }
	void set0e() { E |= 0x01; }
	void set0h() { H |= 0x01; }
	void set0l() { L |= 0x01; }
	void set0HL() { _setHL(0x01); }
	void set0a() { A |= 0x01; }

	// 0xc8
	void set1b() { B |= 0x02; }
	void set1c() { C |= 0x02; }
	void set1d() { D |= 0x02; }
	void set1e() { E |= 0x02; }
	void set1h() { H |= 0x02; }
	void set1l() { L |= 0x02; }
	void set1HL() { _setHL(0x02); }
	void set1a() { A |= 0x02; }

	// 0xd0
	void set2b() { B |= 0x04; }
	void set2c() { C |= 0x04; }
	void set2d() { D |= 0x04; }
	void set2e() { E |= 0x04; }
	void set2h() { H |= 0x04; }
	void set2l() { L |= 0x04; }
	void set2HL() { _setHL(0x04); }
	void set2a() { A |= 0x04; }

	// 0xd8
	void set3b() { B |= 0x08; }
	void set3c() { C |= 0x08; }
	void set3d() { D |= 0x08; }
	void set3e() { E |= 0x08; }
	void set3h() { H |= 0x08; }
	void set3l() { L |= 0x08; }
	void set3HL() { _setHL(0x08); }
	void set3a() { A |= 0x08; }

	// 0xe0
	void set4b() { B |= 0x10; }
	void set4c() { C |= 0x10; }
	void set4d() { D |= 0x10; }
	void set4e() { E |= 0x10; }
	void set4h() { H |= 0x10; }
	void set4l() { L |= 0x10; }
	void set4HL() { _setHL(0x10); }
	void set4a() { A |= 0x10; }

	// 0xe8
	void set5b() { B |= 0x20; }
	void set5c() { C |= 0x20; }
	void set5d() { D |= 0x20; }
	void set5e() { E |= 0x20; }
	void set5h() { H |= 0x20; }
	void set5l() { L |= 0x20; }
	void set5HL() { _setHL(0x20); }
	void set5a() { A |= 0x20; }

	// 0xf0
	void set6b() { B |= 0x40; }
	void set6c() { C |= 0x40; }
	void set6d() { D |= 0x40; }
	void set6e() { E |= 0x40; }
	void set6h() { H |= 0x40; }
	void set6l() { L |= 0x40; }
	void set6HL() { _setHL(0x40); }
	void set6a() { A |= 0x40; }

	// 0xf8
	void set7b() { B |= 0x80; }
	void set7c() { C |= 0x80; }
	void set7d() { D |= 0x80; }
	void set7e() { E |= 0x80; }
	void set7h() { H |= 0x80; }
	void set7l() { L |= 0x80; }
	void set7HL() { _setHL(0x80); }
	void set7a() { A |= 0x80; }
};

#endif
