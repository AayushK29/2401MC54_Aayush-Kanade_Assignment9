
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE 4096

int
main(void)
{
  char *p;

  printf("=== Before sbrk ===\n");
  vmprint();

  // Request exactly four pages of additional memory.
  p = sbrk(4 * PGSIZE);

  if(p == (char *)-1){
    printf("pttest: sbrk failed\n");
    exit(1);
  }

  printf("\n=== After sbrk(4 * 4096) ===\n");
  vmprint();

  exit(0);
}
