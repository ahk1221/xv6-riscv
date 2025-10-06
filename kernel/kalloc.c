// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// Superpage allocation structure
struct superrun {
  struct superrun *next;
};

struct {
  struct spinlock lock;
  struct superrun *freelist;
} supermem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
  
  // Initialize superpage allocator
  initlock(&supermem.lock, "supermem");
  
  // Reserve a handful of 2MB-aligned superpages from the top of RAM
  int max_super = 8; // handful
  uint64 top = PGROUNDDOWN((uint64)PHYSTOP);
  // Align down to 2MB boundary
  top = (top >> MEGAPGSHIFT) << MEGAPGSHIFT;
  
  for(int i = 0; i < max_super; i++){
    uint64 start = top - (i+1)*MEGAPGSIZE;
    if(start < (uint64)end)
      break;
    
    // push onto supermem freelist
    struct superrun *sr = (struct superrun*)start;
    sr->next = supermem.freelist;
    supermem.freelist = sr;
  }
  
  // Free the remaining RAM to the 4KB allocator, but skip superpage pool region
  uint64 reserved = (uint64)supermem.freelist ? (top - max_super*MEGAPGSIZE) : top;
  if(reserved < (uint64)end)
    reserved = (uint64)end;
  freerange(end, (void*)reserved);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// Allocate one 2MB-aligned superpage from the pool.
void *
superalloc(void)
{
  acquire(&supermem.lock);
  struct superrun *sr = supermem.freelist;
  if(sr)
    supermem.freelist = sr->next;
  release(&supermem.lock);

  if(sr)
    memset((char*)sr, 5, MEGAPGSIZE);
  return (void*)sr;
}

// Free one 2MB superpage back to the pool.
void
superfree(void *pa)
{
  if(((uint64)pa % MEGAPGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("superfree");
  
  memset(pa, 1, MEGAPGSIZE);
  
  acquire(&supermem.lock);
  ((struct superrun*)pa)->next = supermem.freelist;
  supermem.freelist = (struct superrun*)pa;
  release(&supermem.lock);
}
