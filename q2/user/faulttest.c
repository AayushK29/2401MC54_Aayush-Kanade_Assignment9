#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define PGSIZE 4096

int
main(void)
{
  char *p;

  // Request 32 pages using the lazy-allocation interface.
  p = sbrklazy(32 * PGSIZE);

  if(p == (char *)-1) {
    printf("faulttest: sbrk failed\n");
    exit(1);
  }

  printf("Reserved 32 pages.\n");

  // Access five different pages for the first time.
  p[0] = 'A';
  p[PGSIZE] = 'B';
  p[2 * PGSIZE] = 'C';
  p[3 * PGSIZE] = 'D';
  p[4 * PGSIZE] = 'E';

  printf("Accessed five different pages.\n");

  int faults = getfaults();

  printf("Page faults handled: %d\n", faults);

  if(faults == 5)
    printf("PASS: exactly five page faults handled.\n");
  else
    printf("FAIL: expected 5 page faults.\n");

  exit(0);
}
