#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "6502.h"
#include "memory.h"

class ramimage: public Memory::Device
{
public:
  virtual void operator= (byte c) { _mem[_acc] = c; }
  virtual operator byte () { return _mem[_acc]; }

  ramimage(int b): Device(b / Memory::page_size), _mem(new byte[b]), _b(b) {}

  int load(const char *file);

  const char *name() const { return "ramimage"; }
  
private:
  byte *_mem;
  int _b;
};

int ramimage::load(const char *file) {
  int f = open(file, O_RDONLY);
  if (f < 0) {
    perror("open");
    return -1;
  }
  for (int i = 0; i < _b; i++) {
    byte c;
    int n = read(f, &c, 1);
    if (n == 0) {
      fprintf(stderr, "short file: %d\n", i);
      break;
    }
    _mem[i] = c;
  }
  close(f);
  return 0;
}

void status(const char *fmt, ...) {
  char tmp[128];
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp, sizeof(tmp), fmt, args);
  printf(tmp);
  va_end(args);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "Usage: %s image.bin\n", argv[0]);
    return -1;
  }
  
  Memory memory;
  ramimage ram(65536);
  if (0 > ram.load(argv[1]))
    return -1;
  memory.put(ram, 0x0000);

  // initialise vectored start address
  memory[0xfffc] = 0x00;
  memory[0xfffd] = 0x04;
  Memory::address opc = 0xfffc;

  jmp_buf ex;
  r6502 cpu(&memory, &ex, status);
  cpu.reset();

  if (!setjmp(ex))
    while (true) {
      Memory::address pc = cpu.run(1111);
      if (pc == opc)
        break;
      opc = pc;
    }

  printf(cpu.status());
}
