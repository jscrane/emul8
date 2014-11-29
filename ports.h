#ifndef __PORTS_H__
#define __PORTS_H__

class i8080; 

class PortDevice {
public:
	virtual void out(byte p, byte v, i8080 *cpu) =0;
	virtual byte in(byte p, i8080 *cpu) =0;
};

#endif
