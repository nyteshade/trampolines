#include <stdio.h>

void dump_tramp(const char *label, const void *p, unsigned len)
{
  const unsigned char *b = (const unsigned char*)p;
  unsigned i;
  printf("%s @ %p (%u bytes):", label, p, len);
  for (i = 0; i < len; ++i) {
    if ((i % 16) == 0) printf("\n  ");
    printf("%02X ", b[i]);
  }
  printf("\n");
}