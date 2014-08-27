#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
  int bin = 0, a = 1;
  
  if (strcmp(argv[a], "-b") == 0) {
    bin = 1;
    a++;
  }
  char *file = argv[a++], *name = argv[a];
  FILE *f = fopen(file, "r");

  printf("static const unsigned char %s[] = {", name);

  for (int i = 0; ; i++) {
    unsigned c;
    int n;
    if (bin)
      n = fread(&c, 1, 1, f);
    else
      n = fscanf(f, "%02x", &c);
    if (!(i % 8))
      printf("\n\t");
    if (n == 0)
      break;
    printf("0x%02x, ", c);
  }
  
  printf("};\n");
  fclose(f);
}
