#ifndef __PORTS_H__
#define __PORTS_H__

template<class P>
class PortDevice {
public:
	virtual void out(byte p, byte v, P *cpu) =0;
	virtual byte in(byte p, P *cpu) =0;
};

#endif
