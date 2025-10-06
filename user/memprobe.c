#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char **argv)
{
  const int chunkSize = 4096; // 1 page
  int pagesAllocated = 0;
  int targetPages = 0;

  if (argc == 2) {
    targetPages = atoi(argv[1]);
  }

  // Keep growing the heap one page at a time until sbrk fails
  while(1) {
    if (targetPages && pagesAllocated >= targetPages)
      break;

    void *p = sbrk(chunkSize);
    if (p == (void*)-1) {
      printf("sbrk failed after %d pages (about %d KB)\n", pagesAllocated, pagesAllocated * 4);
      break;
    }

    // Touch one byte per page to force allocation
    char *c = (char*)p;
    c[0] = 1;
    pagesAllocated++;

    if ((pagesAllocated % 1024) == 0) {
      printf("allocated %d pages (%d MB) so far\n", pagesAllocated, (pagesAllocated * 4) / 1024);
    }
  }

  printf("total allocated: %d pages (%d MB)\n", pagesAllocated, (pagesAllocated * 4) / 1024);
  exit(0);
}


